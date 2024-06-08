// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UserFacingMapData.generated.h"

class ULevelSequence;

/**
 * A collection of map-specific properties and map metadata.
 */
USTRUCT()
struct FFrontEndMapData
{
	GENERATED_BODY()

public:

	/** The user-friendly name of this map. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "User-Facing Name")
	FText UserFacingName;

	/** The image used to represent this map in the options of the custom game menu. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> CustomGameImage;
};



/**
 * Data asset used to define metadata properties for each playable map. This data is displayed to users when maps are
 * shown; e.g. the custom game menu. This is only loaded in menus.
 */
UCLASS()
class PROJECTCRASH_API UUserFacingMapData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Level IDs mapped to the corresponding map data. */
	UPROPERTY(EditDefaultsOnly, Meta = (AllowedTypes="Map"))
	TMap<FPrimaryAssetId, FFrontEndMapData> MapData;
};
