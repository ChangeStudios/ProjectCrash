// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Animation/CrashAnimTypes.h"
#include "Engine/DataAsset.h"
#include "EquipmentAnimationData.generated.h"

class UAnimInstance;
class UAnimMontage;

/**
 * Data that defines the animation of an equipment set while equipped.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentAnimationData : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Animation instance used by the character while this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly, DisplayName="Animation Instance")
	TObjectPtr<UAnimInstance> EquipmentAnimInstance;

	/** Animation played when equipping this equipment set. */
	UPROPERTY(EditDefaultsOnly, DisplayName="Animation Instance")
	FAnimPerspectivePair EquipAnimation;
	
};
