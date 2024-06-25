// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UserFacingGameModeData.generated.h"

/**
 * User-facing format of a game setting's value. All game setting values are stored as floats internally.
 */
UENUM(BlueprintType)
enum class EGameSettingValueType : uint8
{
	// Integer
	WholeNumber,
	// Float
	Decimal,
	// Bool
	Boolean
};



/**
 * Defines how a game mode setting appears to users.
 */
USTRUCT(BlueprintType)
struct FUserFacingGameModeSetting
{
	GENERATED_BODY()

	/** The type of value users may enter for this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGameSettingValueType ValueType = EGameSettingValueType::WholeNumber;

	/** User-facing name of this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	/** This setting's initial value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DefaultValue = 0.0f;

	/** Minimum value allowed for this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "ValueType == EGameSettingValueType::WholeNumber || ValueType == EGameSettingValueType::Decimal", EditConditionHides = "true"))
	float ValueMin = 0.0f;

	/** Maximum value allowed for this setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "ValueType == EGameSettingValueType::WholeNumber || ValueType == EGameSettingValueType::Decimal", EditConditionHides = "true"))
	float ValueMax = 1.0f;

	/** The units with which this setting is displayed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (EditCondition = "ValueType == EGameSettingValueType::WholeNumber || ValueType == EGameSettingValueType::Decimal", EditConditionHides = "true"))
	FText Units;
};



/**
 * Immutable game mode metadata displayed to users in places like custom game lobbies.
 */
UCLASS(BlueprintType, NotBlueprintable, Const, DisplayName = "User-Facing Game Mode Data")
class PROJECTCRASH_API UUserFacingGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Game mode.

public:

	/** The actual game mode associated with this user facing game mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode", Meta = (AllowedTypes = "CrashGameModeData"))
	FPrimaryAssetId GameModeData;

	/** The maps that can be played in this game mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode", Meta = (AllowedTypes = "Map"))
	TArray<FPrimaryAssetId> PlayableMaps;



	// Metadata.

public:

	/** The name of this game mode. Usually a combination of its user-facing title and subtitle; e.g.
	 * "Team Deathmatch." */
	UPROPERTY(EditDefaultsOnly, Category = "Metadata", DisplayName = "User-Facing Name")
	FText UserFacingName;



	// User interface.

public:

	/** Image used when this game mode is listed in the list of game modes in the custom game menu. */
	UPROPERTY(EditDefaultsOnly, Category = "User Interface", DisplayName = "Custom Game Listing Banner", Meta = (AssetBundles = "MainMenu"))
	TSoftObjectPtr<UTexture2D> CustomGameBanner_Listing;

	/** Images used when this game mode is selected in a custom game. */
	UPROPERTY(EditDefaultsOnly, Category = "User Interface", DisplayName = "Selected Custom Game Banner", Meta = (AssetBundles = "MainMenu"))
	TSoftObjectPtr<UTexture2D> CustomGameBanner_Selected;



	// Game mode settings.

public:

	/** Game mode-specific properties that can be set by users in custom games; e.g. "StartingLives" or
	 * "RoundsToWin." The values players set these as are given to the game as game options arguments. */
	UPROPERTY(EditDefaultsOnly, Category = "User-Facing Game Mode Settings")
	TMap<FString, FUserFacingGameModeSetting> UserFacingGameModeSettings;
};
