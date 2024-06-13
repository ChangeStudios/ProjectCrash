// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "CustomGameDefinition.generated.h"

/**
 * Mutable properties used to create a custom game. UUserFacingGameModeData is immutable, so users manipulate this data,
 * with rules specified by UUserFacingGameModeData, to define their custom game's properties.
 */
UCLASS(BlueprintType)
class UCustomGameDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	/** The game mode to play. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Mode", Meta = (AllowedTypes="CrashGameModeData"))
	FPrimaryAssetId GameMode;

	/** The map to play on. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Mode", Meta = (AllowedTypes="Map"))
	FPrimaryAssetId Map;

	/** Additional, optional parameters to pass to game options. This is where customizable, game mode-specific rules
	 * would be passed. E.g. "StartingLives", "TargetScore", "MatchDuration", etc. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game Mode")
	TMap<FString, FString> ExtraArgs;
};
