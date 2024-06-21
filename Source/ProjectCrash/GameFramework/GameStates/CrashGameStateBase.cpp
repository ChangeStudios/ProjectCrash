// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameStates/CrashGameStateBase.h"

#include "CrashGameplayTags.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Development/CrashDeveloperSettings.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFramework/Data/UserFacingMapData.h"
#include "GameFramework/GameModes/Game/CrashGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerStates/CrashPlayerState_DEP.h"

const FName ACrashGameStateBase::NAME_ActorFeatureName("CrashGameState");

void ACrashGameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();

	// Start listening for initialization state changes to this actor.
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	/* Define a lambda that wraps this actor's OnActorInitStateChanged. Used for listening to other actors' init state
	 * changes, such as the player states. */
	ActorInitStateChangedDelegate = FActorInitStateChangedDelegate::CreateWeakLambda(this, [this](const FActorInitStateChangedParams& Params)
	{
		this->OnActorInitStateChanged(Params);
	});
}

void ACrashGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the initialization state.
	ensure(TryToChangeInitState(CrashGameplayTags::TAG_InitState_WaitingForData));
	CheckDefaultInitialization();
}

void ACrashGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
		// Game mode data must be loaded.
		// TODO: ALL players in the session must have the current map loaded. Check for any session disconnects
		return IsValid(GameModeData);
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
	// WaitingForData loads and replicates the game mode data.
	if (!CurrentState.IsValid() && DesiredState == CrashGameplayTags::TAG_InitState_WaitingForData)
	{
		// Only set game mode data on the server.
		if (HasAuthority())
		{
			// If game mode data has already been set and loaded, do nothing.
			if (GameModeData)
			{
				UE_LOG(LogTemp, Error, TEXT("Game mode data has already been loaded: [%s]."), *GameModeData->GetName());
				return;
			}

#if WITH_EDITOR
			// In the editor, override the game mode data using developer settings.
			if (const UCrashDeveloperSettings* DeveloperSettings = GetDefault<UCrashDeveloperSettings>())
			{
				// Only override the game mode data if an overriding game mode data asset has been set.
				if (DeveloperSettings->GameModeDataOverride.IsValid())
				{
					SetGameModeData(DeveloperSettings->GameModeDataOverride);
				}
			}
#endif // WITH_EDITOR

			// Try to retrieve the game mode data path from the 
			if (GameModeDataPath.ToSoftObjectPath().IsNull())
			{
				// TODO: Retrieve game mode data path from game options.
			}

			/* We MUST have the game mode data by this point (even if it isn't loaded yet). The game cannot properly
			 * initialize without the game mode data. */
			if (GameModeDataPath.ToSoftObjectPath().IsNull())
			{
				UE_LOG(LogCrash, Fatal, TEXT("Could not retrieve game mode data! This is non-recoverable; the game will not be able to initialize. Ensure the game mode data is set in game options, or overridden in developer settings."));
			}
		}
	}
}

void ACrashGameStateBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. one of our components), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		UE_LOG(LogTemp, Error, TEXT("Received change from [%s]: changed to [%s]"), *Params.FeatureName.ToString(), *Params.FeatureState.ToString());

		CheckDefaultInitialization();
	}
}

void ACrashGameStateBase::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain({ STATE_WAITING_FOR_DATA, STATE_INITIALIZING, STATE_GAMEPLAY_READY });
}

void ACrashGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	/* Start listening for init state changes from the new player state. Calls this actor's OnActorInitStateChanged a
	 * change is received. */
	if (ACrashPlayerState_DEP* CrashPS = Cast<ACrashPlayerState_DEP>(PlayerState))
	{
		// ActorInitStateChangedHandles.Add(CrashPS, GetComponentManager()->RegisterAndCallForActorInitState(CrashPS, CrashPS->GetFeatureName(), FGameplayTag(), ActorInitStateChangedDelegate, false));
	}
}

void ACrashGameStateBase::SetGameModeData(FPrimaryAssetId GameModeDataId)
{
	// check(InGameModeDataPath.ToSoftObjectPath().IsValid());
	//
	// // Game mode data can only be set from the server. It is replicated to clients.
	// if (!HasAuthority())
	// {
	// 	return;
	// }
	//
	// // We should only ever be setting game mode data once.
	// if (GameModeDataPath.ToSoftObjectPath().IsValid())
	// {
	// 	UE_LOG(LogCrash, Error, TEXT("Trying to set game mode data [%s] on game state, when game mode data has already been set to [%s]. If you want to load data from game options, make sure there is no overriding game data selected in the developer settings."), *InGameModeDataPath.GetAssetName(), *GameModeDataPath.GetAssetName());
	// 	return;
	// }
	//
	// // Update the game mode data path.
	// GameModeDataPath = InGameModeDataPath;
	// ForceNetUpdate();
	//
	// // Manually call the OnRep to load the game mode data on the server.
	// OnRep_GameModeDataPath();
}

const UCrashGameModeData* ACrashGameStateBase::GetGameModeData() const
{
	if (GameModeData.Get())
	{
		return GameModeData;
	}
	else if (GameModeDataPath.ToSoftObjectPath().IsNull())
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

void ACrashGameStateBase::CallOrRegister_OnGameModeDataLoaded(FGameModeDataLoadedSignature::FDelegate&& InGameModeDataLoadedDelegate)
{
}

void ACrashGameStateBase::OnRep_GameModeDataPath()
{
	// Ensure a valid path.
	const FSoftObjectPath& AssetPath = GameModeDataPath.ToSoftObjectPath();
	check(AssetPath.IsValid());

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameModeData Object"), STAT_GameModeData, STATGROUP_LoadTime);

	UE_LOG(LogCrash, Log, TEXT("Loading GameModeData: [%s] ..."), *GameModeDataPath.ToString());

	// Load the new game mode data.
	FStreamableManager& Streamable = UCrashAssetManager::Get().GetStreamableManager();
	Streamable.RequestAsyncLoad({AssetPath}, FStreamableDelegate::CreateUObject(this, &ACrashGameStateBase::OnGameModeDataLoaded));
}

void ACrashGameStateBase::OnGameModeDataLoaded()
{
	// Cache and broadcast the loaded game mode data.
	GameModeData = GameModeDataPath.Get();
	GameModeDataLoadedDelegate.Broadcast(GameModeData);

	SCOPE_LOG_TIME_IN_SECONDS(TEXT("		... GameModeData loaded!"), nullptr);

	/* Game mode data being loaded is a requisite for the initialization chain. Once loaded, check if we are ready to
	 * continue initialization. */
	CheckDefaultInitialization();
}

void ACrashGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashGameStateBase, GameModeDataPath);
}
