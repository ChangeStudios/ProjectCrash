// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "CrashAssetManager.generated.h"

/**
 * Handles loading of primary assets and maintains references to game-specific assets.
 */
UCLASS()
class PROJECTCRASH_API UCrashAssetManager : public UAssetManager
{
	GENERATED_BODY()

	// Class management.

public:

	/** Default constructor. */
	UCrashAssetManager();

	/** CrashAssetManager singleton getter. */
	static UCrashAssetManager& Get();



	// Loading.

protected:

	/** Performs initial object loading. */
	virtual void StartInitialLoading() override;
};
