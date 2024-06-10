// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HealthAttributeBaseValues.generated.h"

/**
 * Data asset that defines the base values for attributes in the health attribute set.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UHealthAttributeBaseValues : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The base value for the Health attribute. */
	UPROPERTY(EditDefaultsOnly, Category = "Stateful Attributes")
	float BaseHealth = 100.0f;

	/** The base value for the MaxHealth attribute. */
	UPROPERTY(EditDefaultsOnly, Category = "Stateful Attributes")
	float BaseMaxHealth = 100.0f;


	/** The base value for the Damage meta attribute. */
	UPROPERTY(EditDefaultsOnly, Category = "Meta Attributes")
	float BaseDamage = 0.0f;
	
	/** The base value for the Healing meta attribute. */
	UPROPERTY(EditDefaultsOnly, Category = "Meta Attributes")
	float BaseHealing = 0.0f;
};
