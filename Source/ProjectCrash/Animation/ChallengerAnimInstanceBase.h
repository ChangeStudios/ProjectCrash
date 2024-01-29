// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ChallengerAnimInstanceBase.generated.h"

class UCharacterAnimData;
class UEquipmentAnimationData;

/**
 * The base animation instance for all challenger characters.
 */
UCLASS()
class PROJECTCRASH_API UChallengerAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

	// Animation data.

public:

	/** Updates this animation instance's current animation data. */
	UFUNCTION(BlueprintCallable, Category = "Animation|Data")
	void UpdateAnimData(UCharacterAnimData* NewAnimData);

protected:

	/** The animation data currently being used by this animation instance. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Data")
	TObjectPtr<UCharacterAnimData> CurrentAnimationData;
};
