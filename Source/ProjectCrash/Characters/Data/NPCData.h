// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPCData.generated.h"

class UHealthAttributeBaseValues;

/**
 * Data used to define the default properties of an NPC, such as its default attribute values.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "NPC Data", ShortToolTip = "Data used to define the default properties of a non-playable character."))
class PROJECTCRASH_API UNPCData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Attributes.

public:

	/** The default values to which health attributes are initialized. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TObjectPtr<UHealthAttributeBaseValues> HealthAttributeBaseValues;
};
