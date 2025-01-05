// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CrashDeveloperSettings.generated.h"

class UCrashCameraModeBase;
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

	/** The default camera mode is determined by the pawn data used. If set, this camera mode will be used instead. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	TSubclassOf<UCrashCameraModeBase> CameraModeOverride;

	/** Whether to skip straight to the gameplay phase in PIE. Enabling this skips the challenger selection, "waiting
	 * for players," and countdown phases. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	bool bSkipToGameplay = false;

	/** Disable all ability cooldowns. Note that cooldowns will still appear in HUD, but won't restrict the activation
	 * of abilities. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	bool bDisableCooldowns = false;

	/** If true, all messages broadcast via the gameplay message subsystem will be logged. */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Messages")
	bool bLogGameplayMessages = false;

	/** Disables developer settings pop-ups (e.g. "Developer Settings Override Pawn Data: ..."). */
	UPROPERTY(Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Editor")
	bool bSuppressDeveloperSettingsMessages = false;
};
