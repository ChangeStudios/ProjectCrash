// Copyright Samuel Reitich 2024.


#include "GameFramework/GameStates/CrashGameStateBase.h"

#include "CrashGameplayTags.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/Data/CrashGameModeData.h"
#include "GameFramework/GameModes/Game/CrashGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

const FName ACrashGameStateBase::NAME_ActorFeatureName("CrashGameState");

void ACrashGameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();

	// Start listening for initialization state changes to this actor.
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
}

void ACrashGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the initialization state.
	ensure(TryToChangeInitState(CrashGameplayTags::TAG_InitState_WaitingForData));
	CheckDefaultInitialization();

#if WITH_EDITOR
	// TODO: Check developer settings for overriding game mode data.
#endif // WITH_EDITOR
}

void ACrashGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from initialization states.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool ACrashGameStateBase::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can always transition to our initial state: WaitingForData.
	if (!CurrentState.IsValid() && DesiredState == CrashGameplayTags::TAG_InitState_WaitingForData)
	{
		return true;
	}
	else if (CurrentState == CrashGameplayTags::TAG_InitState_WaitingForData && DesiredState == CrashGameplayTags::TAG_InitState_Initializing)
	{
		// TODO: Game data must be loaded and ALL players in the session must have the current map loaded. Check for any session disconnects.
		return true;
	}
	else if (CurrentState == CrashGameplayTags::TAG_InitState_Initializing && DesiredState == CrashGameplayTags::TAG_InitState_GameplayReady)
	{
		// TODO: All players have been assigned to teams.
		return false;
	}

	return false;
}

void ACrashGameStateBase::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
}

void ACrashGameStateBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. one of our components), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void ACrashGameStateBase::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();

	/* Attempts to progress through the state chain if requirements for the next state are met, determined by
	 * CanChangeInitState. */
	ContinueInitStateChain(CrashGameplayTags::StateChain);
}

void ACrashGameStateBase::SetGameModeDataPath(TSoftObjectPtr<const UCrashGameModeData> InGameModeDataPath)
{
	check(InGameModeDataPath.IsValid());

	// Game mode data can only be set from the server. It is replicated to clients.
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// We should only ever be setting game mode data once.
	if (GameModeDataPath.IsValid())
	{
		UE_LOG(LogCrash, Error, TEXT("Trying to set game mode data [%s] on game state, when game mode data has already been set to [%s]. If you want to load data from game options, make sure there is no overriding game data selected in the developer settings."), *InGameModeDataPath.GetAssetName(), *GameModeDataPath.GetAssetName());
		return;
	}

	// Update the game mode data path.
	GameModeDataPath = InGameModeDataPath;
	ForceNetUpdate();

	// Load the new game mode data on the server.
	// TODO: Load game mode data via asset manager with OnGameModeDataLoaded as a callback.
}

const UCrashGameModeData* ACrashGameStateBase::GetGameModeData() const
{
	if (GameModeData.Get())
	{
		return GameModeData;
	}
	else if (!GameModeDataPath.IsValid())
	{
		UE_LOG(LogGameState, Error, TEXT("Attempted to retrieve game mode data before data has been set and replicated."));
		return nullptr;
	}
	else
	{
		UE_LOG(LogGameState, Error, TEXT("Attempted to retrieve game mode data before data has been loaded."));
		return nullptr;
	}
}

void ACrashGameStateBase::OnRep_GameModeDataPath()
{
	// TODO: Async load data with OnGameModeDataLoaded as a callback.
}

void ACrashGameStateBase::OnGameModeDataLoaded()
{
	/* Game mode data being valid is a requisite for the initialization chain. Once loaded, check if we are ready to
	 * continue initialization. */
	CheckDefaultInitialization();
}

void ACrashGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashGameStateBase, GameModeDataPath);
}
