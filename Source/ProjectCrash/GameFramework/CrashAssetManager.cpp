// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashAssetManager.h"

#include "CrashLogging.h"
#include "UnrealEngine.h"
#include "Engine/Engine.h"
#include "GameFramework/GlobalGameData.h"
#include "Misc/ScopedSlowTask.h"
#include "UI/Data/UserInterfaceData.h"

static FAutoConsoleCommand CVarDumpLoadedAssets
(
	TEXT("Crash.DumpLoadedAssets"),
	TEXT("Shows all data assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UCrashAssetManager::DumpLoadedAssets(UDataAsset::StaticClass());
	})
);

namespace Crash
{
	static bool bLogAssetLoads = false;
	static FAutoConsoleVariableRef CVarLogAssetLoads
	(
		TEXT("Crash.LogAssetLoads"),
		bLogAssetLoads,
		TEXT("Whether manual asset loads should be logged.")
	);
}

UCrashAssetManager::UCrashAssetManager()
{
	GlobalGameData = nullptr;
	MainMenuUIData = nullptr;
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

void UCrashAssetManager::StartInitialLoading()
{
	SCOPED_BOOT_TIMING("UCrashAssetManager: StartInitialLoading");

	Super::StartInitialLoading();

	// Load global game data.
	GetGlobalGameData();
}

const UGlobalGameData& UCrashAssetManager::GetGlobalGameData()
{
	// Retrieve the global game data if it's loaded.
	if (GlobalGameData)
	{
		return *GlobalGameData;
	}

	// Synchronously load the global game data if it's not loaded.
	return *CastChecked<const UGlobalGameData>(LoadGlobalGameData(UGlobalGameData::StaticClass(), GlobalGameDataPath, UGlobalGameData::StaticClass()->GetFName()));
}

const UUserInterfaceData& UCrashAssetManager::GetMainMenuUIData()
{
	// Retrieve the loaded main menu UI data. If it's not loaded, perform a blocking load.
	// return GetOrSyncLoadGameDataFromPath<UUserInterfaceData>(EGlobalGameDataType::MainMenuUIData, MainMenuUIDataPath);
	return *MainMenuUIData;
}

UPrimaryDataAsset* UCrashAssetManager::LoadGlobalGameData(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GlobalGameData Object"), STAT_GlobalGameData, STATGROUP_LoadTime);

	if (!DataPath.IsNull())
	{

#if WITH_EDITOR
		// Profiling.
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("CrashEditor", "BeginLoadingGlobalGameDataTask", "LoadingGameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif // WITH_EDITOR

		UE_LOG(LogCrash, Log, TEXT("Loading GlobalGameData: [%s] ..."), *DataPath.ToString());

		/* The editor calls this on-demand in PostLoad, meaning this can get stuck being called recursively. To prevent
		 * that, we force a sync load in the editor. */
		if (GIsEditor)
		{
			Asset = DataPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		// In the game, perform a blocking load.
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);
				Asset = CastChecked<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}

		SCOPE_LOG_TIME_IN_SECONDS(TEXT("		... GlobalGameData loaded!"), nullptr);
	}

	if (Asset)
	{
		GlobalGameData = CastChecked<UGlobalGameData>(Asset);
	}
	else
	{
		// The game cannot continue without the global game data.
		UE_LOG(LogCrash, Fatal, TEXT("Failed to load GlobalGameData asset at [%s], type [%s]. This is not recoverable. Check the config file to ensure you have the correct global game data configured, and make sure you have added the global game data to the asset manager's assets in the project settings."), *DataPath.ToString(), *PrimaryAssetType.ToString());
	}

	return Asset;
}

UObject* UCrashAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		// Log the asset load if desired.
		if (Crash::bLogAssetLoads)
		{
			TUniquePtr<FScopeLogTime> LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]."), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		// Use the streamable manager to load the asset if the asset manager has been loaded.
		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if the asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

#if WITH_EDITOR
void UCrashAssetManager::DumpLoadedAssets(UClass* ClassToFilter)
{
	UE_LOG(LogCrash, Log, TEXT("== Start Dumping Loaded Asset Registry Assets ==="));
	UE_LOG(LogCrash, Log, TEXT("================================================="));

	uint32 DumpedAssetsCount = 0;

	// Get all loaded assets in the asset registry and filter for ClassToFilter, if it was given.
	TArray<FAssetData> OutAssets;
	Get().GetAssetRegistry().GetAllAssets(OutAssets);
	OutAssets = ClassToFilter ? OutAssets.FilterByPredicate([ClassToFilter](const FAssetData Asset)
	{
		return Asset.GetAsset()->IsA(ClassToFilter);
	}) : OutAssets;

	// Log the filtered assets.
	UE_LOG(LogCrash, Log, TEXT("Size: %llu"), Get().GetAssetRegistry().GetAllocatedSize());
	for (auto OutAsset : OutAssets)
	{
		UE_LOG(LogCrash, Log, TEXT("	* Asset Registry: [%s]"), *OutAsset.AssetName.ToString());

		DumpedAssetsCount++;
	}

	UE_LOG(LogCrash, Log, TEXT("================================================="));
	UE_LOG(LogCrash, Log, TEXT("Total Asset Registry Assets Dumped: [%i]."), DumpedAssetsCount);
	UE_LOG(LogCrash, Log, TEXT("========= Finish Asset Registry Assets =========="));
}
#endif // WITH_EDITOR
