// Copyright Samuel Reitich. All rights reserved.


#include "Animation/FirstPersonCharacterAnimInstance.h"

#include "GameFramework/PawnMovementComponent.h"

// Inverse of the minimum framerate required to perform spring calculations.
#define MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS 0.1f /* 10 FPS */
/* Universal multiplier applied to spring model stiffness. Used to scale stiffness values to a more intuitive range for
 * designers. */
#define GLOBAL_SPRING_STIFFNESS_SCALE 35.0f

void UFirstPersonCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// We can't safely update our animation data without a valid pawn owner.
	if (!TryGetPawnOwner())
	{
		return;
	}

	// Update sway data.
	UpdateMovementSwayData();
	UpdateAimSwayData();
}

void UFirstPersonCharacterAnimInstance::UpdateAimData(float DeltaSeconds)
{
	const FRotator PreviousAimRotation = AimRotation;

	Super::UpdateAimData(DeltaSeconds);

	const FRotator RotationDelta = (AimRotation - PreviousAimRotation);

	// Aim speed.
	AimSpeedRightLeft = FMath::Clamp((RotationDelta.Yaw * SafeInvertDeltaSeconds(DeltaSeconds)), -MaxAimSpeed, MaxAimSpeed);
	AimSpeedUpDown = FMath::Clamp((RotationDelta.Pitch * SafeInvertDeltaSeconds(DeltaSeconds)), -MaxAimSpeed, MaxAimSpeed);
}

void UFirstPersonCharacterAnimInstance::UpdateMovementSwayData()
{
	APawn* OwningPawn = TryGetPawnOwner();

	/* MaxAimSpeed is used to bound speed used for aim sway. For movement sway, we instead use the pawn's maximum
	 * movement speed as the bound. */
	const float MaxMovementSpeed = OwningPawn->GetMovementComponent()->GetMaxSpeed();

	// Calculate the target values for the movement sway springs: the current speed normalized to the maximum speed.
	float SpringTargetForwardBackward = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.X, 0.0f, MaxMovementSpeed);
	SpringTargetForwardBackward *= MovementSwayForwardBackwardData.InterpSpeed;

	float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.Y, 0.0f, MaxMovementSpeed);
	SpringTargetRightLeft *= MovementSwayRightLeftData.InterpSpeed;

	// Calculate the new spring values.
	CurrentSpringMoveForwardBackward = UpdateFloatSpringInterp
	(
		CurrentSpringMoveForwardBackward,
		SpringTargetForwardBackward,
		SpringStateMoveForwardBackward,
		MovementSwayForwardBackwardData
	);

	CurrentSpringMoveRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringMoveRightLeft,
		SpringTargetRightLeft,
		SpringStateMoveRightLeft,
		MovementSwayRightLeftData
	);
}

void UFirstPersonCharacterAnimInstance::UpdateAimSwayData()
{
	// Calculate the target values for the aim sway springs: the current aim speed normalized to the maximum aim speed.
	float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange(AimSpeedRightLeft, 0.0f, MaxAimSpeed);
	SpringTargetRightLeft *= AimSwayRightLeftData.InterpSpeed;

	float SpringTargetUpDown = UKismetMathLibrary::NormalizeToRange(AimSpeedUpDown, 0.0f, MaxAimSpeed);
	SpringTargetUpDown *= AimSwayUpDownData.InterpSpeed;

	// Calculate the new spring values.
	CurrentSpringAimRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringAimRightLeft,
		SpringTargetRightLeft,
		SpringStateAimRightLeft,
		AimSwayRightLeftData
	);

	CurrentSpringAimUpDown = UpdateFloatSpringInterp
	(
		CurrentSpringAimUpDown,
		SpringTargetUpDown,
		SpringStateAimUpDown,
		AimSwayUpDownData
	);
}

float UFirstPersonCharacterAnimInstance::UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FFloatSpringModelData& SpringData)
{
	const float DeltaSeconds = GetDeltaSeconds();

	// Don't bother performing spring calculations at low frame-rates.
	if (DeltaSeconds > MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS)
	{
		return SpringCurrent;
	}

	/** Apply an arbitrary multiplier to the spring's stiffness value. This scales viable spring stiffness values to a
	 * more intuitive range of 0-100 when adjusting spring model data. */
	const float EffectiveStiffness = SpringData.Stiffness * GLOBAL_SPRING_STIFFNESS_SCALE;

	// Perform the spring calculation with the given data.
	return UKismetMathLibrary::FloatSpringInterp
	(
		SpringCurrent,
		SpringTarget,
		SpringState,
		EffectiveStiffness,
		SpringData.CriticalDampingFactor,
		DeltaSeconds,
		SpringData.Mass,
		1.0f,
		false
	);
}
