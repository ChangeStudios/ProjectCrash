// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameStates/CrashGameState.h"

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/Data/CrashGameModeData.h"
#include "GameFramework/Data/GameFeatureActionSet.h"
#include "Net/UnrealNetwork.h"

const FName ACrashGameState::NAME_ActorFeatureName("CrashGameState");

ACrashGameState::ACrashGameState()
{
}

void ACrashGameState::SetGameModeData(FPrimaryAssetId GameModeDataId)
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
			UE_LOG(LogTemp, Error, TEXT("Loading game feature at %s"), *PluginURL);
			// TODO: Notify GameFeatureManager of plugin activation
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
	// Update the number of game features loading.
	NumGameFeaturePluginsLoading--;

	UE_LOG(LogTemp, Error, TEXT("Finished loading plugin"));

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

	// Continue initialization.
}

void ACrashGameState::OnRep_CurrentGameModeData()
{
	// Start loading the game mode data when it's received by clients.
	StartGameModeLoad();
}

void ACrashGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashGameState, CurrentGameModeData);
}