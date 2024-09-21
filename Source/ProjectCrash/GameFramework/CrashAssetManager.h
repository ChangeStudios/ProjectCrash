// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "CrashAssetManager.generated.h"

class UGlobalGameData;
class UUserInterfaceData;

/**
 * The asset manager singleton for this project. Used for manually loading and unloading assets as needed, and
 * providing global access to certain data.
 */
UCLASS(Config = Game)
class PROJECTCRASH_API UCrashAssetManager : public UAssetManager
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashAssetManager();

	/** Global getter for the asset manager singleton. */
	static UCrashAssetManager& Get();



	// Initialization.

protected:

	/** Loads the global game data on startup. */
	virtual void StartInitialLoading() override;



	// Assets.

// Accessors.
public:

	/** Returns the global game data. Sync loads the data if it isn't loaded already. */
	const UGlobalGameData& GetGlobalGameData();

// Data.
protected:

	/** Global game data asset to use. */
	UPROPERTY(Config)
	TSoftObjectPtr<UGlobalGameData> GlobalGameDataPath;

	/** Loaded version of global game data. */
	UPROPERTY(Transient)
	TObjectPtr<UGlobalGameData> GlobalGameData;



	// Asset loading.

// Game data.
protected:

	/** Synchronously loads the global game data at the given path. Throws a fatal exception if we cannot successfully
	 * load the global game data. */
	UPrimaryDataAsset* LoadGlobalGameData(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataPath, FPrimaryAssetType PrimaryAssetType);

// Synchronous asset loading.
public:

	/** Retrieves the given asset. Synchronously loads the asset if it's not loaded. */
	template <typename AssetType>
	static AssetType* GetOrLoadAsset(const TSoftObjectPtr<AssetType>& AssetPointer);

	/** Retrieves the given class. Synchronously loads the class if it's not loaded. */
	template <typename AssetType>
	static TSubclassOf<AssetType> GetOrLoadClass(const TSoftClassPtr<AssetType>& AssetPointer);

protected:

	/** Synchronously loads the given data asset via a blocking load. */
	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);



	// Utils.

public:

#if WITH_EDITOR
	/** Logs all assets currently loaded by the asset registry. */
	static void DumpLoadedAssets(UClass* ClassToFilter = nullptr);
#endif // WITH_EDITOR
};



template <typename AssetType>
AssetType* UCrashAssetManager::GetOrLoadAsset(const TSoftObjectPtr<AssetType>& AssetPointer)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		// Try to get the asset if it's already loaded.
		LoadedAsset = AssetPointer.Get();

		// Sync load the asset if it isn't loaded.
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]."), *AssetPointer.ToString());
		}
	}

	return LoadedAsset;
}

template <typename AssetType>
TSubclassOf<AssetType> UCrashAssetManager::GetOrLoadClass(const TSoftClassPtr<AssetType>& AssetPointer)
{
	TSubclassOf<AssetType> LoadedClass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		// Try to get the class if it's already loaded.
		LoadedClass = AssetPointer.Get();

		// Sync load the class if it isn't loaded.
		if (!LoadedClass)
		{
			LoadedClass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedClass, TEXT("Failed to load asset class [%s]."), *AssetPointer.ToString());
		}
	}

	return LoadedClass;
}
