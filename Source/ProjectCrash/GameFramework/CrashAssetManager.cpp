// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashAssetManager.h"

#include "CrashLogging.h"
#include "UnrealEngine.h"
#include "Engine/Engine.h"
#include "Misc/ScopedSlowTask.h"
#include "UI/UserInterfaceData.h"

UCrashAssetManager::UCrashAssetManager()
{
}

UCrashAssetManager& UCrashAssetManager::Get()
{
	check(GEngine);

	// Retrieve the world's asset manager.
	if (UCrashAssetManager* AssetManager = Cast<UCrashAssetManager>(GEngine->AssetManager))
	{
		return *AssetManager;
	}

	UE_LOG(LogEngine, Fatal, TEXT("Project default asset manager is not set to UCrashAssetManager."));

	// Fallback for safety. This should never happen.
	return *NewObject<UCrashAssetManager>();
}

const UUserInterfaceData& UCrashAssetManager::GetMainMenuUIData()
{
	// Retrieve the loaded main menu UI data. If it's not loaded, perform a blocking load.
	return GetOrSyncLoadGameDataFromPath<UUserInterfaceData>(EGlobalGameDataType::MainMenuUIData, MainMenuUIDataPath);
}

UPrimaryDataAsset* UCrashAssetManager::SyncLoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, EGlobalGameDataType DataType, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);

	if (!DataClassPath.IsNull())
	{

#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("CrashEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPie = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPie);
#endif // WITH_EDITOR

		UE_LOG(LogCrash, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("	... GameData loaded!"), nullptr);

		// Perform a blocking load of the given data asset.
		Asset = DataClassPath.LoadSynchronous();
		LoadPrimaryAssetsWithType(PrimaryAssetType);
	}

	// If the asset was loaded, cache it in the game data map.
	if (Asset)
	{
		GameDataMap.Add(DataType, Asset);
	}
	else
	{
		UE_LOG(LogCrash, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}

#if WITH_EDITOR
void UCrashAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogCrash, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	uint32 DumpedAssetsCount = 0;

	// Log every global game data asset that is currently loaded.
	for (auto LoadedData : Get().GameDataMap)
	{
		UE_LOG(LogCrash, Log, TEXT("	* [%s]"), *GetNameSafe(LoadedData.Value));

		DumpedAssetsCount = LoadedData.Value->IsValidLowLevelFast() ? DumpedAssetsCount + 1 : DumpedAssetsCount;
	}

	UE_LOG(LogCrash, Log, TEXT("Total Assets Dumped: [%i]."), DumpedAssetsCount);
	UE_LOG(LogCrash, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}
#endif // WITH_EDITOR
