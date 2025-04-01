// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionJumpForce.h"
#include "AbilityTask_ApplyRootMotionAdditiveJumpForce.generated.h"

/**
 * Applies force to character's movement.
 *
 * This is a rewrite of UAbilityTask_ApplyRootMotionJumpForce, which is very difficult to subclass.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_ApplyRootMotionAdditiveJumpForce : public UAbilityTask_ApplyRootMotionJumpForce
{
	GENERATED_BODY()

	/** Applies a jump force to the character's movement without disabling their XY input. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_ApplyRootMotionAdditiveJumpForce* ApplyRootMotionAdditiveJumpForce(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		FRotator InRotation,
		float InDistance,
		float InHeight,
		float InDuration,
		float InMinimumLandedTriggerTime,
		bool bInFinishOnLanded,
		ERootMotionFinishVelocityMode VelocityOnFinishMode,
		FVector SetVelocityOnFinish,
		float ClampVelocityOnFinish,
		UCurveVector* InPathOffsetCurve,
		UCurveFloat* InTimeMappingCurve);

protected:

	virtual void SharedInitAndApply() override;
};
