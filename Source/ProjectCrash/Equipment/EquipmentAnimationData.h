// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimData/CharacterAnimData.h"
#include "EquipmentAnimationData.generated.h"

class UAnimInstance;
class UAnimMontage;
class UBlendSpace;
class UBlendSpace1D;

/**
 * Data that extends the default character animation data with data utilized for equipment sets.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentAnimationData : public UCharacterAnimData
{
	GENERATED_BODY()

	// First-person animations.

// Actions.
public:

	/** First-person animation played when equipping this equipment set. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Actions", DisplayName = "Equip")
	TObjectPtr<UAnimMontage> Equip_FPP;



	// Third-person animations.

// Actions.
public:

	/** Third-person animation played when equipping this equipment set. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Actions", DisplayName = "Equip")
	TObjectPtr<UAnimMontage> Equip_TPP;
};
