// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameState.h"

#include "CrashGameplayTags.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFramework/GameFeatures/GameFeatureActionSet.h"
#include "GameFramework/GameFeatures/GameFeatureManager.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerStates/CrashPlayerState.h"

const FName ACrashGameState::NAME_ActorFeatureName("CrashGameState");

ACrashGameState::ACrashGameState()
{
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
	Super::EndPlay(EndPlayReason);

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

	// Start unloading the current game mode.
	StartGameModeUnload();
}

bool ACrashGameState::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can also transition to our initial state, WaitingForData, when our current state hasn't been initialized yet.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return true;
	}
	// Transition to Initializing when (1) the game mode data is loaded and (2) all player states are in Initializing.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// The game mode must be loaded (game features are activated, actions have been executed, etc.).
		const bool bGameModeLoaded = (LoadState == ECrashGameModeLoadState::Loaded);

		// TODO: The number of player states in Initializing must be the number of players in the session.
		bool bAllPlayersReadyToInitialize = true;
		// TODO: Check for disconnects.

		return bGameModeLoaded && bAllPlayersReadyToInitialize;
	}
	// Transition to GameplayReady when all components AND player states are in GameplayReady.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		// All game state components must be GameplayReady.
		bool bAllComponentsReady = Manager->HaveAllFeaturesReachedInitState(const_cast<ACrashGameState*>(this), STATE_GAMEPLAY_READY, GetFeatureName());

		// TODO: players must be GameplayReady.
		bool bAllPlayersGameplayReady = true;
		// TODO: Check for disconnects.

		return bAllComponentsReady && bAllPlayersGameplayReady;
	}

	return false;
}

void ACrashGameState::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
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
}

void ACrashGameState::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain(CrashGameplayTags::StateChain);
}

void ACrashGameState::SetGameModeData(const FPrimaryAssetId& GameModeDataId)
{
	// Load the appropriate game mode data.
	UCrashAssetManager& AssetManager = UCrashAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(GameModeDataId);
	TObjectPtr<UCrashGameModeData> LoadedGameModeData = Cast<UCrashGameModeData>(AssetPath.TryLoad());
	check(LoadedGameModeData);

	// Cache and replicate the loaded game mode data.
	CurrentGameModeData = LoadedGameModeData;

	// Start loading the game mode on the server. The OnRep will start the load for clients.
	StartGameModeLoad();
}

void ACrashGameState::StartGameModeLoad()
{
	check(CurrentGameModeData != nullptr);
	check(LoadState == ECrashGameModeLoadState::Unloaded);

	UE_LOG(LogCrashGameMode, Log, TEXT("Crash Game Mode: Started game mode load. Game Mode: [%s] (%s)"),
		*CurrentGameModeData->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	// Update load state.
	LoadState = ECrashGameModeLoadState::Loading;



	UCrashAssetManager& AssetManager = UCrashAssetManager::Get();

	// Assets whose bundles will be loaded.
	TSet<FPrimaryAssetId> BundleAssetList;

	// Add the game mode data and its action sets to the list of assets whose bundles will be loaded.
	BundleAssetList.Add(CurrentGameModeData->GetPrimaryAssetId());
	for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentGameModeData->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// Determine which bundles to load for the assets in BundleAssetList.
	TArray<FName> BundlesToLoad;
	const ENetMode OwnerNetMode = GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);

	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	// Start loading the new bundles on the specified assets.
	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}



	// Bind the callback to when the assets finish loading. 
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameModeLoadComplete);
	if (!BundleLoadHandle.IsValid() || BundleLoadHandle->HasLoadCompleted())
	{
		// If the assets already finished loading (or didn't have to load anything), immediately invoke the callback.
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		BundleLoadHandle->BindCompleteDelegate(OnAssetsLoadedDelegate);
		BundleLoadHandle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
		{
			OnAssetsLoadedDelegate.ExecuteIfBound();
		}));
	}
}

void ACrashGameState::OnGameModeLoadComplete()
{
	check(CurrentGameModeData != nullptr);
	check(LoadState == ECrashGameModeLoadState::Loading);

	UE_LOG(LogCrashGameMode, Log, TEXT("Crash Game Mode: Completed initial game mode load. Game Mode: [%s] (%s)"),
		*CurrentGameModeData->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));



	// Find the URLs for the game feature plugins needed for this game mode.
	GameFeaturePluginURLs.Reset();

	// Helper lambda for caching the URLs of the given game features.
	auto CollectGameFeaturePluginURLs = [This = this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString OutPluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, OutPluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(OutPluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnGameModeLoadComplete failed to find plugin URL from plugin name [%s] for game mode [%s]. Ignoring..."), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}
	};

	// Collect the game features required by the game mode data.
	CollectGameFeaturePluginURLs(CurrentGameModeData, CurrentGameModeData->GameFeaturesToEnable);

	// Collect the game features required by the game mode's action sets.
	for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentGameModeData->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	// Load and activate the collected game features.
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = ECrashGameModeLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			FString PluginName;
			UGameFeaturesSubsystem::Get().GetPluginNameByURL(PluginURL, PluginName);
			UE_LOG(LogCrashGameMode, Log, TEXT("Loading game feature plugin [%s]..."), *PluginName);

			// Notify the game feature manager that a new plugin was activated.
			UGameFeatureManager::NotifyOfPluginActivation(PluginURL);

			// Load and activate the plugin.
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
	// If no game features need to be loaded, we are finished loading.
	else
	{
		OnGameModeFullLoadComplete();
	}
}

void ACrashGameState::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	FString PluginName;
	UGameFeaturesSubsystem::Get().GetPluginNameByURL(GameFeaturePluginURLs[NumGameFeaturePluginsLoading - 1], PluginName);
	UE_LOG(LogCrashGameMode, Log, TEXT("		... Finished loading game feature plugin [%s]."), *PluginName);

	// Update the number of game features loading.
	NumGameFeaturePluginsLoading--;

	// Wait until ALL game features have finished loading to finish loading the game mode.
	if (NumGameFeaturePluginsLoading == 0)
	{
		OnGameModeFullLoadComplete();
	}
}

void ACrashGameState::OnGameModeFullLoadComplete()
{
	check(LoadState != ECrashGameModeLoadState::Loaded);

	UE_LOG(LogCrashGameMode, Log, TEXT("Crash Game Mode: Game mode load complete. Game Mode: [%s] (%s)"),
		*CurrentGameModeData->GetPrimaryAssetId().ToString(),
		*GetClientServerContextString(this));

	// Update load state.
	LoadState = ECrashGameModeLoadState::ExecutingActions;



	// Execute the game mode's startup actions.
	FGameFeatureActivatingContext Context;

	// Only apply to our world context, if set.
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	// Helper lambda for activating a list of game feature actions.
	auto ActivateActionsFromList = [&Context](const TArray<UGameFeatureAction*>& Actions)
	{
		for (UGameFeatureAction* Action : Actions)
		{
			if (Action != nullptr)
			{
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	// Activate the game mode data's actions.
	ActivateActionsFromList(CurrentGameModeData->Actions);

	// Activate the game mode data's action sets' actions.
	for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentGameModeData->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateActionsFromList(ActionSet->Actions);
		}
	}



	// Update the load state.
	LoadState = ECrashGameModeLoadState::Loaded;

	// Broadcast that the game mode finished loading.
	CrashGameModeLoadedDelegate.Broadcast(CurrentGameModeData);
	CrashGameModeLoadedDelegate.Clear();

	// Continue initialization, now that the game mode is loaded.
	CheckDefaultInitialization();
}

void ACrashGameState::StartGameModeUnload()
{
	// Deactivate active game feature plugins in FILO.
	for (int32 i = GameFeaturePluginURLs.Num(); i > 0; i--)
	{
		const FString& PluginURL = GameFeaturePluginURLs[i - 1];
		FString PluginName;
		UGameFeaturesSubsystem::Get().GetPluginNameByURL(PluginURL, PluginName);

		if (UGameFeatureManager::RequestToDeactivatePlugin(PluginURL))
		{
			/* Deactivate and unload the plugin. It would be best to do this asynchronously and to wait for
			 * deactivation to finish before unloading, but this is called in the game state's EndPlay, so the game
			 * state won't be around to unload the plugin. */
			UE_LOG(LogCrashGameMode, Log, TEXT("Unloading game feature plugin [%s]."), *PluginName);
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
			UGameFeaturesSubsystem::Get().UnloadGameFeaturePlugin(PluginURL);
		}
	}

	// Deactivate and unload active game feature actions.
	if (LoadState == ECrashGameModeLoadState::Loaded)
	{
		LoadState = ECrashGameModeLoadState::Deactivating;

		// Track any asynchronous action deactivations we may have to wait for.
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		// Deactivate and unload actions.
		FGameFeatureDeactivatingContext Context(TEXT(""), [this](FStringView) { this->OnActionDeactivationCompleted(); });

		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		// Helper lambda for deactivating lists of actions.
		auto DeactivateActionsFromList = [&Context](const TArray<UGameFeatureAction*>& Actions)
		{
			for (UGameFeatureAction* Action : Actions)
			{
				if (Action != nullptr)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
					Action->OnGameFeatureUnloading();
				}
			}
		};

		// Deactivate the current game mode data's actions.
		DeactivateActionsFromList(CurrentGameModeData->Actions);

		// Deactivate the current game mode's action sets' actions.
		for (const TObjectPtr<UGameFeatureActionSet>& ActionSet : CurrentGameModeData->ActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateActionsFromList(ActionSet->Actions);
			}
		}

		// Check for any actions that deactivate asynchronously.
		NumExpectedPausers = Context.GetNumPausers();

		if (NumExpectedPausers > 0)
		{
			UE_LOG(LogCrashGameMode, Error, TEXT("Actions that have asynchronous deactivation aren't fully supported."));
		}

		/* Attempt to immediately invoke final game mode unload. This specific call will only succeed if there are no
		 * asynchronous deactivations or plugins to unload, which are checked here. */
		OnGameModeUnloadComplete();
	}
}

void ACrashGameState::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	++NumObservedPausers;

	// Attempt final game mode unload, which will also check if all plugins are deactivated and unloaded.
	OnGameModeUnloadComplete();
}

void ACrashGameState::OnGameFeaturePluginUnloadComplete(const UE::GameFeatures::FResult& Result)
{
	// Update the number of plugins unloading.
	NumGameFeaturePluginsUnloading--;

	// Invoke the final game mode unload, which will check if all plugins have been unloaded.
	OnGameModeUnloadComplete();
}

void ACrashGameState::OnGameModeUnloadComplete()
{
	// All actions must be fully deactivated and unloaded.
	if (NumObservedPausers != NumExpectedPausers)
	{
		return;
	}

	UE_LOG(LogCrashGameMode, Log, TEXT("Game mode unloaded."));

	// Final game mode unload.
	LoadState = ECrashGameModeLoadState::Unloaded;
	CurrentGameModeData = nullptr;
}

void ACrashGameState::OnRep_CurrentGameModeData()
{
	// Start loading the game mode data when it's received by clients.
	StartGameModeLoad();
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

void ACrashGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashGameState, CurrentGameModeData);
}