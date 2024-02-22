// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DynamicAbilityDataAsset.generated.h"

/**
 * Base class for data asset classes used to store dynamic data for ability sets. Dynamic data primarily consists of
 * cosmetic information, such as which mesh to spawn for certain abilities, depending on the currently equipped skin.
 */
UCLASS(Abstract, BlueprintType)
class PROJECTCRASH_API UDynamicAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
};
