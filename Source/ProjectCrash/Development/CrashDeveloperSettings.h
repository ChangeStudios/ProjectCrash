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

#if WITH_EDITOR

public:

	/** Notifies the user of any active developer overrides. */
	PROJECTCRASH_API void OnPlayInEditorBegin() const;

	/** Applies developer settings when updated. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Applies developer settings when reloaded. */
	virtual void PostReloadConfig(class FProperty* PropertyThatWasLoaded) override;

	/** Applies developer settings on initialization. */
	virtual void PostInitProperties() override;

private:

	/** Performs any necessary actions for developer settings. */
	void ApplySettings();

#endif // WITH_EDITOR



	// Settings.

public:

	/** The game mode data is determined by the game options. If set, this data will be used instead. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode", Meta = (AllowedTypes = "CrashGameModeData"))
	FPrimaryAssetId GameModeDataOverride;

	/** Each player's pawn data is determined by the game mode, or by an optional character selection screen. If set,
	 * this data will be used for all players instead. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode", Meta = (AllowedTypes = "PawnData"))
	FPrimaryAssetId PawnDataOverride;

	/** If true, all messages broadcast via the gameplay message subsystem will be logged. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Messages")
	bool bLogGameplayMessages = false;
};
