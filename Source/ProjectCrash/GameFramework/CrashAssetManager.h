// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/DataAsset.h"
#include "CrashAssetManager.generated.h"

class UCrashGameModeData;
class UUserInterfaceData;

/**
 * This project's asset manager singleton. Used for loading and unloading data assets to only keep the relevant assets
 * in memory.
 */
UCLASS()
class PROJECTCRASH_API UCrashAssetManager : public UAssetManager
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashAssetManager();

	/** Global getter for the current asset manager singleton. */
	static UCrashAssetManager& Get();



	// Game mode asset loading.

public:

	/** Returns or synchronously loads the current game mode's GM data. */
	const UCrashGameModeData& GetGameModeData() const;

	/** Returns or synchronously loads the current game mode's UI data. */
	const UUserInterfaceData& GetUIData() const;



	// Synchronous asset loading utilities.



	// Asynchronous asset loading utilities.



	// Utils.

public:

#if WITH_EDITOR
	/** Logs all asset currently loaded by the asset manager. */
	static void DumpLoadedAssets();
#endif // WITH_EDITOR
};
