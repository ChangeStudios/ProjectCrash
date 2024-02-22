// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StaticAbilityDataAsset.generated.h"

/**
 * Base class for data asset classes used to store static data for ability sets.
 *
 * Ability data is grouped by character, rather than ability sets themselves. E.g. abilities in the Knight challenger's
 * default ability set and the abilities in the Knight challenger's sword/shield ability set would both have their
 * static data defined in a single "Knight Ability Data" data asset.
 *
 * In the future, this may provide utilities for more efficiently loading static data. It is also possible these may be
 * made dynamic to implement user-defined settings for custom games.
 */
UCLASS(Abstract, BlueprintType, Const)
class PROJECTCRASH_API UStaticAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
};
