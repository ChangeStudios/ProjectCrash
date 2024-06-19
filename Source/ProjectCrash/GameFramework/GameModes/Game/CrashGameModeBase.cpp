// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/Game/CrashGameModeBase.h"

#include "CrashGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameStates/CrashGameStateBase.h"
#include "Player/PlayerStates/CrashPlayerState_DEP.h"

const FName ACrashGameModeBase::NAME_ActorFeatureName("CrashGameMode");

ACrashGameModeBase::ACrashGameModeBase()
{
	GameStateClass = ACrashGameStateBase::StaticClass();
	PlayerStateClass = ACrashPlayerState_DEP::StaticClass();
}

void ACrashGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();

	FActorInitStateChangedDelegate ActorInitStateChangedDelegate = FActorInitStateChangedDelegate::CreateWeakLambda(this,
	[this](const FActorInitStateChangedParams& Params)
	{
		this->OnActorInitStateChanged(Params);
	});

	// Start listening for changes to the game state's initialization state.
	ACrashGameStateBase* CrashGS = GetGameState<ACrashGameStateBase>();
	ActorInitStateChangedHandles.Add(CrashGS, GetComponentManager()->RegisterAndCallForActorInitState(CrashGS, CrashGS->GetFeatureName(), FGameplayTag(), ActorInitStateChangedDelegate, false));
}

void ACrashGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the initialization state.
	ensure(TryToChangeInitState(CrashGameplayTags::TAG_InitState_WaitingForData));
	CheckDefaultInitialization();
}

void ACrashGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop listening to any other actors' initialization state changes.
	UGameFrameworkComponentManager* Manager = GetComponentManager();
	for (auto ActorHandlePair : ActorInitStateChangedHandles)
	{
		if (ActorHandlePair.Value.IsValid())
		{
			Manager->UnregisterActorInitStateDelegate(ActorHandlePair.Key, ActorHandlePair.Value);
		}
	}
	ActorInitStateChangedHandles.Reset();

	// Unregister from initialization states.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool ACrashGameModeBase::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can always transition to our initial state: WaitingForData.
	if (!CurrentState.IsValid() && DesiredState == CrashGameplayTags::TAG_InitState_WaitingForData)
	{
		return true;
	}
	else if (CurrentState == CrashGameplayTags::TAG_InitState_WaitingForData && DesiredState == CrashGameplayTags::TAG_InitState_Initializing)
	{
		// TODO: Game state and ALL player controllers must be in the Initializing state.
		return Manager->HaveAllFeaturesReachedInitState(GameState, CrashGameplayTags::TAG_InitState_Initializing);
	}
	else if (CurrentState == CrashGameplayTags::TAG_InitState_Initializing && DesiredState == CrashGameplayTags::TAG_InitState_GameplayReady)
	{
		// TODO: ALL connected player states must be in the GameplayReady state.
		return false;
	}

	return false;
}

void ACrashGameModeBase::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

void ACrashGameModeBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. the game state), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void ACrashGameModeBase::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();

	/* Attempts to progress through the state chain if requirements for the next state are met, determined by
	 * CanChangeInitState. */
	ContinueInitStateChain({ STATE_WAITING_FOR_DATA, STATE_INITIALIZING, STATE_GAMEPLAY_READY });
}

UClass* ACrashGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
