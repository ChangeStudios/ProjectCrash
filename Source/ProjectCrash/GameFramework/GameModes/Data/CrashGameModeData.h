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
	// A setting that is enabled or disabled.
	OnOff
};



/**
 * 
 */
USTRUCT(BlueprintType)
struct FCustomGameSettingBehavior
{
	GENERATED_BODY()

// Setting data.
public:

	/** The value type of this variable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ECustomSettingType> CustomSettingType = None;

    /** The user-facing name of this setting. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType != 0", EditConditionHides = true))
    FString DisplayedName = "";

	/** The value to which this setting will be initialized in the settings menu. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "CustomSettingType != 0", EditConditionHides = true))
	float InitialValue = 0.0f;

// Numeric data.
public:

	/** Units used by this setting. Can be left empty to disable units. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	FString Units = "";

	/** Whether this variable can be a decimal (i.e. if it's an int or a float). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	bool bDecimal = true;

	/** Whether a slider can be used to set this variable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	bool bEnableSlider = true;

	/** The step values used by this variable's slider, if it has one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "(CustomSettingType == 1 || CustomSettingType == 2) && bEnableSlider", EditConditionHides = true))
	float SliderStep = 0.1f;

	/** The minimum value to which this variable can be set by the user. Defines the scale of the slider if it's
	 * enabled. Value will be floored if bDecimal is false. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	float ValueMin = 0.0f;

	/** The maximum value to which this variable can be set by the user. Defines the scale of the slider if it's
	 * enabled. Value will be floored if bDecimal is false. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Numeric", Meta = (EditCondition = "CustomSettingType == 1 || CustomSettingType == 2", EditConditionHides = true))
	float ValueMax = 1.0f;
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

	/** Whether to enable overtime for this game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	bool bEnableOvertime;

	/** The maximum amount of time for which overtime will be played. When this time is reached, the match will be
	 * ended with the game mode's tie-breaking rules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", Meta = (Units = "s"))
	float MaximumOvertimeTime;

	/** How long to wait in the match after a team has won before traveling back to the menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	float EndMatchTime;



	// Teams.

public:

	/** Whether to use a distinct fresnel for each team. This is usually true for team-based game modes, and false for
	 * FFA modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Use Hostile Team Fresnels?")
	bool bUseHostileTeamFresnels;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TObjectPtr<UTexture2D> CustomGameActiveImage;

	/** Image used to represent this game mode in the game mode selection pop-up in the custom game menu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TObjectPtr<UTexture2D> CustomGameOptionImage;



	// Custom game settings.

protected:

	/** Defines which variables can be overridden by players in custom games. These variables will appear in the
	 * custom game menu's "settings" menu.
	 *
	 * Note: Variables MUST use their name from C++. Editor-only display names will not work.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Game Settings", Meta = (TitleProperty = CustomSettingType))
	TMap<FName, FCustomGameSettingBehavior> CustomGameSettingsDefinitions;
};
