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
 *
 * TODO: Change this to a bunch of animation sequences and use it to drive a universal animation instance.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentAnimationData : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Animation instance used by the character while this equipment set is equipped. */
	UPROPERTY(EditDefaultsOnly, DisplayName="Animation Instance")
	TSubclassOf<UAnimInstance> EquipmentAnimInstance;

	/** Animation played when equipping this equipment set. */
	UPROPERTY(EditDefaultsOnly, DisplayName="Equip Animation")
	FAnimPerspectivePair EquipAnimation;
	
};
