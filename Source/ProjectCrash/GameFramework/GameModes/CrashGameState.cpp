// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameState.h"

#include "CrashGameplayTags.h"
#include "GameModeManagerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "Player/PlayerStates/CrashPlayerState.h"

const FName ACrashGameState::NAME_ActorFeatureName("CrashGameState");

ACrashGameState::ACrashGameState()
{
	// Create the component responsible for handling the current game mode.
	GameModeManagerComponent = CreateDefaultSubobject<UGameModeManagerComponent>(TEXT("GameModeManagerComponent"));
}

void ACrashGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();

	/* Initialize the actor init state callback to call OnActorInitStateChanged on this actor. This is used to trigger
	 * this actor's OnActorInitStateChanged function when other actors' init states change. */
	ActorInitStateChangedDelegate = FActorInitStateChangedDelegate::CreateWeakLambda(this, [this](const FActorInitStateChangedParams& Params)
	{
		this->OnActorInitStateChanged(Params);
	});
}

void ACrashGameState::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for initialization state changes on this actor (e.g. its components).
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Initialize this actor's initialization state.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();
}

void ACrashGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this actor as an initialization state feature.
	UnregisterInitStateFeature();

	// Stop listening for changes to players' initialization states.
	UGameFrameworkComponentManager* ComponentManager = GetComponentManager();
	check(ComponentManager);
	for (auto PlayerHandlePair : PlayerStateInitStateChangedHandles)
	{
		if (PlayerHandlePair.Value.IsValid())
		{
			ComponentManager->UnregisterActorInitStateDelegate(PlayerHandlePair.Key, PlayerHandlePair.Value);
			PlayerHandlePair.Value.Reset();
		}
	}
	PlayerStateInitStateChangedHandles.Empty();
	ActorInitStateChangedDelegate.Unbind();

	Super::EndPlay(EndPlayReason);
}

bool ACrashGameState::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can always transition to our initial state when our current state hasn't been initialized yet.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return true;
	}
	// Transition to Initializing when (1) the game mode data is loaded and (2) all player states are in Initializing.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// The game mode must be loaded (game features are activated, actions have been executed, etc.).
		if (!GameModeManagerComponent->IsGameModeLoaded())
		{
			return false;
		}

		// TODO: The number of player states in Initializing must be the number of players in the session.
		bool bAllPlayersReadyToInitialize = true;
		// TODO: Check for disconnects.

		return bAllPlayersReadyToInitialize;
	}
	// Transition to GameplayReady when all components AND player states are in GameplayReady.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		// All game state components must be GameplayReady.
		if (!Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashGameState*>(this), STATE_GAMEPLAY_READY, GetFeatureName()))
		{
			return false;
		}

		// TODO: players must be GameplayReady.
		bool bAllPlayersGameplayReady = true;
		// TODO: Check for disconnects.

		return bAllPlayersGameplayReady;
	}

	return false;
}

void ACrashGameState::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/* When transitioning to our initial state, start listening for the game mode to be loaded. The game mode must be
	 * fully loaded for initialization to progress. */
	if (!CurrentState.IsValid() && DesiredState == STATE_INITIALIZING)
	{
		check(GameModeManagerComponent);
		GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateWeakLambda(this, [this](const UCrashGameModeData* GameModeData)
		{
			// Try to progress our init state when the game mode is loaded.
			CheckDefaultInitialization();
		}));
	}
	// When transitioning to GameplayReady, start the game.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		// TODO: Start the game.
		UE_LOG(LogCrashGameMode, Log, TEXT("Game started!"));
	}
}

void ACrashGameState::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. one of our components), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}

	if (Params.FeatureName == ACrashPlayerState::NAME_ActorFeatureName)
	{
		UE_LOG(LogTemp, Error, TEXT("PS Progressed"));
		if (Params.FeatureState == STATE_GAMEPLAY_READY)
		{
			for (APlayerState* PS : PlayerArray)
			{
				if (APlayerController* PC = PS->GetPlayerController())
				{
					if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
					{
						GM->RestartPlayer(PC);
						UE_LOG(LogTemp, Error, TEXT("Restarted"));
					}
				}
			}
		}
	}
}

void ACrashGameState::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain(CrashGameplayTags::StateChain);
}

void ACrashGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	// Listen for changes to players' initialization states, which drive the game state's initialization.
	if (ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(PlayerState))
	{
		if (!CrashPS->IsOnlyASpectator() && !PlayerStateInitStateChangedHandles.Contains(CrashPS))
		{
			UGameFrameworkComponentManager* ComponentManager = GetComponentManager();
			check(ComponentManager);

			/* This will call this actor's OnActorInitStateChanged function when a player state's init state changes,
			 * which will attempt to progress our initialization chain if possible (e.g. if players are now ready). */
			PlayerStateInitStateChangedHandles.Add(CrashPS, ComponentManager->RegisterAndCallForActorInitState(CrashPS, CrashPS->GetFeatureName(), FGameplayTag(), ActorInitStateChangedDelegate, false));
		}
	}
}