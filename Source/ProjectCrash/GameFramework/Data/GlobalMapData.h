// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalMapData.generated.h"

class ULevelSequence;

/**
 * A collection of map-specific properties and map metadata.
 */
USTRUCT()
struct FMapData
{
	GENERATED_BODY()

	// Map properties.

public:

	/** The opening cinematic played for players while they load into the map. */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelSequence> IntroCinematic;



	// Metadata.

public:

	/** The user-friendly name of this map. */
	UPROPERTY(EditDefaultsOnly)
	FName UserFacingName;

	/** The image used to represent this map in the options of the custom game menu. */
	UPROPERTY(EditDefaultsOnly, Meta = (AssetBundles = "MainMenu"))
	TSoftObjectPtr<UTexture2D> CustomGameImage;
};



/**
 * Global data asset used to define properties for each playable map.
 *
 * Includes bundle support for controlling which properties are loaded. E.g. some map textures are only used in menus,
 * and shouldn't be loaded during games.
 */
UCLASS()
class PROJECTCRASH_API UGlobalMapData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Level IDs mapped to the corresponding map data. */
	UPROPERTY(EditDefaultsOnly, Meta = (AllowedTypes="Map"))
	TMap<FPrimaryAssetId, FMapData> MapData;
};
