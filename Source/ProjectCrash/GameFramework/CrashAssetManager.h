// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "UI/UserInterfaceData.h"
#include "CrashAssetManager.generated.h"

class UCrashGameModeData;
class UUserInterfaceData;

/**
 * Types of global game data assets that are manually loaded and unloaded. Only one data asset should ever been loaded
 * for each type at any given time. E.g. in a match, one game mode data asset and one UI data asset will be loaded, but
 * the main menu data asset should not be.
 */
UENUM()
enum class EGlobalGameDataType : uint8
{
	GameModeData,
	UserInterfaceData,
	MainMenuUIData
};

/**
 * This project's asset manager singleton. Used for loading and unloading data assets and providing global access to
 * certain assets.
 */
UCLASS(Config = Game)
class PROJECTCRASH_API UCrashAssetManager : public UAssetManager
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashAssetManager();

	/** Global getter for the current asset manager singleton. */
	static UCrashAssetManager& Get();



	// Assets.

// Accessors.
public:

	/** Returns or loads the global main menu UI data. */
	const UUserInterfaceData& GetMainMenuUIData();

// Data.
protected:

	/** Global main menu UI data asset to use. */
	UPROPERTY(Config)
	TSoftObjectPtr<UUserInterfaceData> MainMenuUIDataPath;

	/** Map maintaining references to loaded assets. Each global game data type should only have one asset loaded for
	 * it at most, at any given time. */
	UPROPERTY(Transient)
	TMap<EGlobalGameDataType, TObjectPtr<UPrimaryDataAsset>> GameDataMap;



	// Asset loading.

public:

	/** Synchronously loads the asset at the given path. If an asset of the given type has already been loaded, it is 
	 * before the new asset is loaded. Caches the new asset in GameDataMap. */
	UPrimaryDataAsset* SyncLoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, EGlobalGameDataType DataType, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:

	/** Retrieves the game data asset at the specific path if it's loaded. Performs a synchronous load otherwise. */
	template <typename GameDataClass>
	const GameDataClass& GetOrSyncLoadGameDataFromPath(EGlobalGameDataType DataType, const TSoftObjectPtr<UPrimaryDataAsset>& DataPath)
	{
		// Retrieve the asset if it is already loaded.
		if (TObjectPtr<UPrimaryDataAsset> const* pResult = GameDataMap.Find(DataType))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Synchronous load.
		return *CastChecked<const GameDataClass>(SyncLoadGameDataOfClass(GameDataClass::StaticClass(), DataType, DataPath, GameDataClass::StaticClass()->GetFName()));
	}



    // Asset unloading.

public:

	bool UnloadGameData(EGlobalGameDataType DataType);


	// Utils.

public:

#if WITH_EDITOR
	/** Logs all assets currently loaded by the asset manager. */
	static void DumpLoadedAssets();
#endif // WITH_EDITOR
};
