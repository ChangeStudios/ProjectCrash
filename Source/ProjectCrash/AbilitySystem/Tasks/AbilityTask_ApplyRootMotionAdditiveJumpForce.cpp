// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/Tasks/AbilityTask_ApplyRootMotionAdditiveJumpForce.h"

#include "GameFramework/RootMotionSource.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

UAbilityTask_ApplyRootMotionAdditiveJumpForce* UAbilityTask_ApplyRootMotionAdditiveJumpForce::ApplyRootMotionAdditiveJumpForce(
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
	UCurveFloat* InTimeMappingCurve)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Duration(InDuration);

	UAbilityTask_ApplyRootMotionAdditiveJumpForce* MyTask = NewAbilityTask<UAbilityTask_ApplyRootMotionAdditiveJumpForce>(OwningAbility, TaskInstanceName);

	MyTask->ForceName = TaskInstanceName;
	MyTask->Rotation = InRotation;
	MyTask->Distance = InDistance;
	MyTask->Height = InHeight;
	MyTask->Duration = FMath::Max(InDuration, KINDA_SMALL_NUMBER); // No zero duration
	MyTask->MinimumLandedTriggerTime = InMinimumLandedTriggerTime * InDuration; // MinimumLandedTriggerTime is normalized
	MyTask->bFinishOnLanded = bInFinishOnLanded;
	MyTask->FinishVelocityMode = VelocityOnFinishMode;
	MyTask->FinishSetVelocity = SetVelocityOnFinish;
	MyTask->FinishClampVelocity = ClampVelocityOnFinish;
	MyTask->PathOffsetCurve = InPathOffsetCurve;
	MyTask->TimeMappingCurve = InTimeMappingCurve;
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UAbilityTask_ApplyRootMotionAdditiveJumpForce::SharedInitAndApply()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && ASC->AbilityActorInfo->MovementComponent.IsValid())
	{
		MovementComponent = Cast<UCharacterMovementComponent>(ASC->AbilityActorInfo->MovementComponent.Get());
		StartTime = GetWorld()->GetTimeSeconds();
		EndTime = StartTime + Duration;

		if (MovementComponent)
		{
			ForceName = ForceName.IsNone() ? FName("AbilityTaskApplyRootMotionAdditiveJumpForce") : ForceName;
			TSharedPtr<FRootMotionSource_JumpForce> JumpForce = MakeShared<FRootMotionSource_JumpForce>();
			JumpForce->InstanceName = ForceName;
			JumpForce->AccumulateMode = ERootMotionAccumulateMode::Additive;
			JumpForce->Priority = 501;
			JumpForce->Duration = Duration;
			JumpForce->Rotation = Rotation;
			JumpForce->Distance = Distance;
			JumpForce->Height = Height;
			JumpForce->Duration = Duration;
			JumpForce->bDisableTimeout = bFinishOnLanded; // If we finish on landed, we need to disable force's timeout
			JumpForce->PathOffsetCurve = PathOffsetCurve;
			JumpForce->TimeMappingCurve = TimeMappingCurve;
			// JumpForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
			JumpForce->FinishVelocityParams.Mode = FinishVelocityMode;
			JumpForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
			JumpForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(JumpForce);
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_ApplyRootMotionJumpForce called in Ability %s with null MovementComponent; Task Instance Name %s."), 
			Ability ? *Ability->GetName() : TEXT("NULL"), 
			*InstanceName.ToString());
	}
}

