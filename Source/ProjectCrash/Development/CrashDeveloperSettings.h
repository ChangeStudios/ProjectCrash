// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CrashDeveloperSettings.generated.h"

class UChallengerData;
class UChallengerSkinData;
class UCrashGameModeData;

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



	// Settings.

public:

	/** The game mode data is determined by the game options. If set, this data will be used instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = Crash, Meta = (AllowedTypes = "CrashGameModeData"))
	FPrimaryAssetId GameModeDataOverride;

	/** Each player's Challenger data is determined by the Challenger they select in the character selection screen.
	 * If set, this data will be used for all players instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = Crash, Meta = (AllowedTypes = "ChallengerData"))
	FPrimaryAssetId ChallengerDataOverride;

	/**
	 * Each player's skin data retrieved through their cloud settings when they join the game. If set, this data will
	 * be used for all players instead.
	 *
	 * Because players' skins must match their Challengers, a Challenger override must also be set to use this.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = Crash, Meta = (AllowedTypes = "ChallengerSkinData"))
	FPrimaryAssetId SkinDataOverride;



	// Editing settings.

public:

#if WITH_EDITOR
	/** Determines when certain properties can be changed. */
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif // WITH_EDITOR
};
