// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashGameModeData.generated.h"

class UCommonActivatableWidget;
class UGA_Death;
class UUserInterfaceData;

/**
 * 
 */
UENUM(BlueprintType)
enum ECustomSettingType : uint8
{
	// This value cannot be changed in custom game settings.
	None,
	// A number (int or float) that appears in a small container. Used for variables that are usually from <= 99.
	NumericSmall,
	// A number (int or float) that appears in a wide container. Used for variables that are usually from >= 100.
	NumericLarge,
	// A value from an enumerator.
	Enumerator
};



/**
 * 
 */
USTRUCT(BlueprintType)
struct FCustomGameSettingBehavior
{
	GENERATED_BODY()

public:

	/** How this variable behaves as a custom game setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ECustomSettingType> CustomSettingType = None;

	/** Whether a slider can be used to set this variable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	bool bEnableSlider = false;

	/** The step values used by this variable's slider, if it has one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "(CustomSettingType == 1 || CustomSettingType == 2) && bEnableSlider", EditConditionHides = true))
	float SliderStep = 0.1f;

	/** The minimum value to which this variable can be set by the user. Defines the scale of the slider. Value will be
	 * floored if the variable is an integer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	float ValueMin = 0.0f;

	/** The maximum value to which this variable can be set by the user. Defines the scale of the slider. Value will be
	 * floored if the variable is an integer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	float ValueMax = 0.0f;

	/** When this variable's value is set to or below ValueMin, this will be displayed instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	FName OverrideMin = NAME_None;

	/** When this variable's value is set to or above ValueMax, this will be displayed instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	FName OverrideMax = NAME_None;
};



/**
 * Data defining the properties of a gameplay-oriented game mode (as opposed to an interface-only game mode). Defines
 * data like players' starting lives or the UI data to use.
 *
 * TODO: Define a way to override variables using custom game rules. E.g. defining a list of structs that define custom
 * settings (e.g. sliders, maximum values, etc.) mapped to the variables they override.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Game Mode Data"))
class PROJECTCRASH_API UCrashGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashGameModeData(const FObjectInitializer& ObjectInitializer);



	// Meta.

public:

	/** The user-facing name of this game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meta", DisplayName = "Display Name")
	FString GameModeDisplayName;



	// Match.

public:

	/**
	 * The maximum amount of time for which this match will be played. When this time is reached, the game mode's
	 * overtime will begin. If the game mode does not implement overtime, the match will be ended with the game mode's
	 * tie-breaking rules.
	 *
	 * Setting this to 0.0 disables this time limit.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", Meta = (ForceUnits = "s"))
	float MaximumMatchTime;

	/**
	 * The maximum amount of time for which overtime will be played. When this time is reached, the match will be
	 * ended with the game mode's tie-breaking rules.
	 *
	 * Setting this to 0.0 disables overtime for this game mode.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", Meta = (Units = "s"))
	float MaximumOvertimeTime;



	// Teams.

public:

	/**
	 * The number of teams required for the match. The game will not automatically start until there are this many
	 * teams, each with TeamSize players.
	 *
	 * This is overridden in custom games, which are started manually, regardless of the number of teams.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Number of Teams")
	uint8 NumTeams;

	/**
	 * The number of players required on each team for the match. The game will not automatically start until there are
	 * NumTeams teams with TeamSize players each.
	 *
	 * This is overridden in custom games, which are started manually, regardless of the teams' sizes.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	uint8 TeamSize;



	// Lives.

public:

	/** The number of lives with which players start games. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lives", DisplayName = "Starting Player Lives", Meta = (UIMin = 1, UIMax = 255))
	uint8 StartingLives;



	// Death.

public:

	/** The default death ability to use in this game mode. This will be activated on ASCs when their avatars die via
	 * running out of health. */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TSubclassOf<UGA_Death> DefaultDeathAbility;

	/** The amount of time to wait between StartDeath and FinishDeath. */
	UPROPERTY(EditDefaultsOnly, Category = "Death", Meta = (Units = "seconds"))
	float DeathDuration;



	// UI.

public:

	/** User interface data that defines the UI generated by this game mode. */
	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TObjectPtr<UUserInterfaceData> UIData;

	/** Image used when this game mode is selected in the custom game menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface", Meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UTexture2D> CustomGameActiveImage;

	/** Image used to represent this game mode in the game mode selection pop-up in the custom game menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface", Meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UTexture2D> CustomGameOptionImage;



	// Custom game settings.

protected:

	/** Defines which variables can be overridden by players in custom games. These variables will appear in the
	 * custom game menu's "settings" menu.
	 *
	 * Note: Variables MUST use their name from C++. Editor-only display names will not work.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Game Settings", Meta = (TitleProperty = CustomSettingType))
	TMap<FName, FCustomGameSettingBehavior> CustomGameSettingsDefinitions;
};
