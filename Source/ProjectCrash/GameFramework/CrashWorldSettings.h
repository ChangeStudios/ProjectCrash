// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "CrashWorldSettings.generated.h"

class UCrashGameModeData;

/**
 * Default world settings for this project. Provides important map-specific data, such as which game mode data to use by
 * default.
 */
UCLASS()
class PROJECTCRASH_API ACrashWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

	// World properties.

public:

	/** Retrieves the game mode data object as a primary asset ID, ensuring that it's not in an unscanned directory. */
	FPrimaryAssetId GetDefaultGameModeData() const;

#if WITH_EDITORONLY_DATA
	/** Forces standalone net mode when playing in the editor. Useful for testing levels that should always be played
	 * in standalone, like the main menu. */
	UPROPERTY(EditDefaultsOnly, Category = PIE)
	bool ForceStandaloneNetMode = false;
#endif // WITH_EDITORONLY_DATA

protected:

	/** The default game mode data used for this level. This is used for levels that don't have another means of
	 * determining their game mode data, such as standalone levels (main menu, practice range, etc.). */
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	TSoftObjectPtr<UCrashGameModeData> DefaultGameModeData;



	// Validation.

public:

#if WITH_EDITOR
	/** Ensures project-specific actors are being used in this level. E.g. CrashPlayerStart instead of PlayerStart. */
	virtual void CheckForErrors() override;
#endif // WITH_EDITOR

};
