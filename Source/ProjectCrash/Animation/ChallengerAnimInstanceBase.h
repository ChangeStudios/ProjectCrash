// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ChallengerAnimInstanceBase.generated.h"

// Animation log channel shorthand.
#define ANIMATION_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAnimation, Verbosity, Format, ##__VA_ARGS__); \
}

class UCharacterAnimData;
class UEquipmentAnimationData;

/**
 * The base animation instance for all challenger character animation instances. Provides utilities that both first-
 * person and third-person animation instances utilize.
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
