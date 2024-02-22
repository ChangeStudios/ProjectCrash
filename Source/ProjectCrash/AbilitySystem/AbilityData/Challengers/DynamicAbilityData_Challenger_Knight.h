// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AbilityData/DynamicAbilityDataAsset.h"
#include "DynamicAbilityData_Challenger_Knight.generated.h"

class UEquipmentSet;

/**
 * Dynamic data for the Knight challenger's abilities.
 */
UCLASS()
class PROJECTCRASH_API UDynamicAbilityData_Challenger_Knight : public UDynamicAbilityDataAsset
{
	GENERATED_BODY()

public:

	// Sword & Shield.

	/** The equipment set to use for the Knight's sword & shield equipment. This should always have the same ability
	 * set and animation data, but may use different actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sword & Shield", DisplayName = "Sword & Shield Equipment Set")
	TObjectPtr<UEquipmentSet> SwordShield_EquipmentSet;
};
