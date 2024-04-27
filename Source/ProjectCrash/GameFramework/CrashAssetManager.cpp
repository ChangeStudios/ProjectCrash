// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashAssetManager.h"

#include "CrashLogging.h"
#include "UnrealEngine.h"
#include "Engine/Engine.h"
#include "GameFramework/GlobalGameData.h"
#include "Misc/ScopedSlowTask.h"
#include "UI/Data/UserInterfaceData.h"

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

const UGlobalGameData& UCrashAssetManager::GetGlobalGameData()
{
	// Retrieve the loaded global game data. If it's not loaded, perform a blocking load.
	return GetOrSyncLoadGameDataFromPath<UGlobalGameData>(EGlobalGameDataType::GlobalGameData, GlobalGameDataPath);
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
		// Unload the existing data type if one is already loaded.
		if (const TObjectPtr<UPrimaryDataAsset>* LoadedAsset = GameDataMap.Find(DataType))
		{
			if (LoadedAsset->GetPathName() == DataClassPath->GetPathName())
			{
				Asset = LoadedAsset->Get();
				return Asset;
			}
			else
			{
				UnloadGameData(DataType);
			}
		}

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

bool UCrashAssetManager::UnloadGameData(EGlobalGameDataType DataType)
{
	// If there is a loaded asset of the given type, unload it.
	if (GameDataMap.Contains(DataType))
	{
		if (const TObjectPtr<UPrimaryDataAsset>* GameDataPtr = GameDataMap.Find(DataType))
		{
			const UPrimaryDataAsset* GameData = *GameDataPtr;
			
			if (GameData->IsValidLowLevel() && GameData->GetPrimaryAssetId().IsValid())
			{
				GameDataMap.Remove(DataType);

				UnloadPrimaryAsset(GameData->GetPrimaryAssetId());
				GEngine->ForceGarbageCollection();
				return true;
			}
		}
	}

	return false;
}

#if WITH_EDITOR
void UCrashAssetManager::DumpLoadedAssets(bool bGameDataOnly, UClass* ClassToFilter)
{
	UE_LOG(LogCrash, Log, TEXT("===== Start Dumping Loaded Game Data Assets ====="));
	UE_LOG(LogCrash, Log, TEXT("================================================="));

	uint32 DumpedAssetsCount = 0;

	// Log every global game data asset that is currently loaded.
	for (auto LoadedData : Get().GameDataMap)
	{
		UE_LOG(LogCrash, Log, TEXT("	* Asset Manager: [%s]"), *GetNameSafe(LoadedData.Value));

		DumpedAssetsCount = LoadedData.Value->IsValidLowLevelFast() ? DumpedAssetsCount + 1 : DumpedAssetsCount;
	}

	UE_LOG(LogCrash, Log, TEXT("================================================="));
	UE_LOG(LogCrash, Log, TEXT("Total Asset Manager Assets Dumped: [%i]."), DumpedAssetsCount);
	UE_LOG(LogCrash, Log, TEXT("============ Finish Game Data Assets ============"));


	if (bGameDataOnly)
	{
		return;
	}


	UE_LOG(LogCrash, Log, TEXT("== Start Dumping Loaded Asset Registry Assets ==="));
	UE_LOG(LogCrash, Log, TEXT("================================================="));

	DumpedAssetsCount = 0;

	// Log every asset that is currently loaded by the asset registry.
	TArray<FAssetData> OutAssets;
	Get().GetAssetRegistry().GetAllAssets(OutAssets);
	UE_LOG(LogCrash, Log, TEXT("Size: %llu"), Get().GetAssetRegistry().GetAllocatedSize());
	for (auto OutAsset : OutAssets)
	{
		if (ClassToFilter && OutAsset.GetAsset()->IsA(ClassToFilter))
		{
			UE_LOG(LogCrash, Log, TEXT("	* Asset Registry: [%s]"), *OutAsset.AssetName.ToString());
		}

		DumpedAssetsCount++;
	}

	UE_LOG(LogCrash, Log, TEXT("================================================="));
	UE_LOG(LogCrash, Log, TEXT("Total Asset Registry Assets Dumped: [%i]."), DumpedAssetsCount);
	UE_LOG(LogCrash, Log, TEXT("========= Finish Asset Registry Assets =========="));
}
#endif // WITH_EDITOR
