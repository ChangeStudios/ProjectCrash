// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/GameModeManagerComponent.h"

#include "CrashGameModeData.h"
#include "CrashGameState.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameFeatures/GameFeatureActionSet.h"
#include "GameFramework/GameFeatures/GameFeatureManager.h"
#include "Net/UnrealNetwork.h"


UGameModeManagerComponent::UGameModeManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UGameModeManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Start unloading the current game mode.
	StartGameModeUnload();
}

void UGameModeManagerComponent::SetGameModeData(const FPrimaryAssetId& GameModeDataId)
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

const UCrashGameModeData* UGameModeManagerComponent::GetGameModeData() const
{
	// Return nullptr if the game mode is not loaded.
	return IsGameModeLoaded() ? CurrentGameModeData : nullptr;
}

const UCrashGameModeData* UGameModeManagerComponent::GetGameModeDataChecked() const
{
	// Assert that the game mode is loaded.
	check(IsGameModeLoaded());
	return CurrentGameModeData;
}

bool UGameModeManagerComponent::IsGameModeLoaded() const
{
	// Game mode data must be set and fully loaded.
	return (CurrentGameModeData != nullptr) && (LoadState == ECrashGameModeLoadState::Loaded);
}

void UGameModeManagerComponent::StartGameModeLoad()
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
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
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

void UGameModeManagerComponent::OnGameModeLoadComplete()
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

void UGameModeManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
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

void UGameModeManagerComponent::OnGameModeFullLoadComplete()
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

	// TODO: Switch to using the CrashGameModeLoaded delegate.
	GetGameState<ACrashGameState>()->CheckDefaultInitialization();
}

void UGameModeManagerComponent::StartGameModeUnload()
{
	UE_LOG(LogCrashGameMode, Log, TEXT("Crash Game Mode: Started game mode unload. (%s)"),
		*GetClientServerContextString(this));

	// Deactivate active game feature plugins in FILO.
	for (int32 i = GameFeaturePluginURLs.Num(); i > 0; i--)
	{
		const FString& PluginURL = GameFeaturePluginURLs[i - 1];
		FString PluginName;
		UGameFeaturesSubsystem::Get().GetPluginNameByURL(PluginURL, PluginName);

		if (UGameFeatureManager::RequestToDeactivatePlugin(PluginURL))
		{
			// Deactivate the plugin.

			/* TODO: The plugin should also be unloaded asynchronously after it finishes being deactivated. This can't
			 * be done here because StartGameModeUnload is called in EndPlay, so the game state will be destroyed
			 * before it can receive any deactivation/unload callbacks. */
			UE_LOG(LogCrashGameMode, Log, TEXT("Deactivating game feature plugin [%s]. (%s)"),
				*PluginName,
				*GetClientServerContextString(this));
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
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
					// TODO: Unload the action.
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

void UGameModeManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	NumObservedPausers++;

	// Attempt final game mode unload, which will also check if all plugins are deactivated and unloaded.
	OnGameModeUnloadComplete();
}

void UGameModeManagerComponent::OnGameModeUnloadComplete()
{
	// All actions must be fully deactivated and unloaded.
	if (NumObservedPausers != NumExpectedPausers)
	{
		return;
	}

	UE_LOG(LogCrashGameMode, Log, TEXT("Crash Game Mode: Game mode fully unloaded. (%s)"),
		*GetClientServerContextString(this));

	// Final game mode unload.
	LoadState = ECrashGameModeLoadState::Unloaded;
	CurrentGameModeData = nullptr;
}

void UGameModeManagerComponent::OnRep_CurrentGameModeData()
{
	// Start loading the game mode data when it's received by clients.
	StartGameModeLoad();
}

void UGameModeManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameModeData);
}
