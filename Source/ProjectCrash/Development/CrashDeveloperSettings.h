// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CrashDeveloperSettings.generated.h"

class UCrashGameModeData;
class UPawnData;

/**
 * Developer settings for development and testing.
 */
UCLASS(Config = EditorPerProjectUserSettings, MinimalAPI)
class UCrashDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

	// Construction.
	
public:

	/** Default constructor. */
	UCrashDeveloperSettings();

	/** Defines the category under which these settings appear in project settings. */
	virtual FName GetCategoryName() const override;



	// Initialization.

public:

#if WITH_EDITOR
	/** Notifies the user of any active developer overrides. TODO: Forcefully loads the pawn data if necessary. */
	PROJECTCRASH_API void OnPlayInEditorBegin() const;
#endif // WITH_EDITOR



	// Settings.

public:

	/** The game mode data is determined by the game options. If set, this data will be used instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Crash", Meta = (AllowedTypes = "CrashGameModeData"))
	FPrimaryAssetId GameModeDataOverride;

	/** Each player's pawn data is determined by the game mode, or by an optional character selection screen. If set,
	 * this data will be used for all players instead.
	 *
	 * TODO: Make FPrimaryAssetId
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Crash", Meta = (AllowedTypes = "PawnData"))
	FPrimaryAssetId PawnDataOverride;
};
