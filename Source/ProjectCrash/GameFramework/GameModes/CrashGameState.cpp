// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameState.h"

#include "CrashGameplayTags.h"
#include "GameModeManagerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "Player/CrashPlayerState.h"
#include "Editor/UnrealEd/Classes/Settings/LevelEditorPlaySettings.h"

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
}

void ACrashGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Attempt to progress initialization when the game mode data finishes loading.
	GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateWeakLambda(this, [this](const UCrashGameModeData* CrashGameModeData)
	{
		CheckDefaultInitialization();
	}));
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
	// Unregister this actor from the initialization state framework.
	UnregisterInitStateFeature();

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
	/* Transition to Initializing when (1) the game mode data is loaded, (2) all components are in Initializing, and
	 * (3) the expected number of players have joined the game. */
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// The game mode must be loaded (game features are activated, actions have been executed, etc.).
		if (!GameModeManagerComponent->IsGameModeLoaded())
		{
			return false;
		}

		// All game state components must have reached Initializing.
		if (!Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashGameState*>(this), STATE_GAMEPLAY_READY, GetFeatureName()))
		{
			return false;
		}

		// All players must be connected.
		return GetNumExpectedPlayers() == PlayerArray.Num();
	}
	// Transition to GameplayReady when all components are in GameplayReady.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		// All game state components must have reached GameplayReady.
		return Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashGameState*>(this), STATE_GAMEPLAY_READY, GetFeatureName());

		// @Note: We may want to check the player array again to make sure no one has disconnected or gone inactive.
	}

	return false;
}

void ACrashGameState::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/* When transitioning to our initial state, start listening for the game mode to be loaded. The game mode must be
	 * fully loaded for initialization to progress. */
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		check(GameModeManagerComponent);
		GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate::CreateWeakLambda(this, [this](const UCrashGameModeData* GameModeData)
		{
			// Try to progress our init state when the game mode is loaded.
			CheckDefaultInitialization();
		}));
	}
	// When transitioning to Initializing, take players out of the loading screen.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// TODO: Take players out of the loading screen.
	}
	// When transitioning to GameplayReady, start the game.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		if (HasAuthority())
		{
			// TODO: Start the game.
			UE_LOG(LogCrashGameMode, Log, TEXT("Game started!"));
		}
	}
}

void ACrashGameState::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (i.e. a game state component), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
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

	// Try to progress initialization when a new player joins (potentially the last player we're waiting for).
	CheckDefaultInitialization();
}

int32 ACrashGameState::GetNumExpectedPlayers() const
{
	// During PIE, use the "Number of Players" setting from the multiplayer options.
	if (GetWorld()->IsPlayInEditor())
	{
		const ULevelEditorPlaySettings* PlayInSettings = GetDefault<ULevelEditorPlaySettings>();
		int32 PlayNumberOfClients = 0;
		PlayInSettings->GetPlayNumberOfClients(PlayNumberOfClients);
		return PlayNumberOfClients;
	}

	// TODO: Read session settings for the number of players in the session.

	// TODO: Check for disconnects.

	return 0;
}

int32 ACrashGameState::GetNumActivePlayers() const
{
	int32 ActivePlayerCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		// Don't count spectators.
		if (PS->IsSpectator())
		{
			continue;
		}

		// Don't count inactive players.
		if (PS->IsInactive())
		{
			continue;
		}

		// Count all other players.
		ActivePlayerCount++;
	}

	return ActivePlayerCount;
}