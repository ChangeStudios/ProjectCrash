// Copyright Samuel Reitich. All rights reserved.


#include "Animation/FirstPersonCharacterAnimInstance.h"

#include "Characters/CrashCharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

/**
 * The maximum absolute speed, in degrees/second, with which aim sway will be calculated. This acts as a bound for
 * scaling aim sway with speed, preventing issues like arms completely flying away when turning too quickly.
 *
 * This also acts as the range to which aim speed will be normalized.
 */
#define MAX_AIM_SPEED 720.0f

/** The maximum aim speed for up/down aiming is halved because characters' pitch has half the range of their yaw:
 * (-90 -> 90) vs. (0 -> 360). */
#define MAX_AIM_SPEED_UP_DOWN (MAX_AIM_SPEED / 2.0f)

/** Inverse of the minimum framerate required to perform spring calculations. */
#define MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS 0.1f /* 10 FPS */

/** Universal multiplier applied to spring model stiffness. Used to scale stiffness values to a more intuitive
 * range for designers. */
#define GLOBAL_SPRING_STIFFNESS_SCALE 35.0f


UFirstPersonCharacterAnimInstance::UFirstPersonCharacterAnimInstance() :
	AimSpeedRightLeft(0.0f),
	AimSpeedUpDown(0.0f),
	CurrentSpringMoveForwardBackward(0.0f),
	CurrentSpringMoveRightLeft(0.0f),
	CurrentSpringAimRightLeft(0.0f),
	CurrentSpringAimUpDown(0.0f),
	CurrentSpringFalling(0.0f)
{
}

void UFirstPersonCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// We can't safely update our animation data without a valid pawn owner.
	if (!TryGetPawnOwner())
	{
		return;
	}

	// Wait for our movement component and movement mode to be initialized.
	UCrashCharacterMovementComponent* CharMovementComp = GetCrashCharacterMovementComponent();
	if (!CharMovementComp || (CharMovementComp->MovementMode == MOVE_None))
	{
		return;
	}

	// Update sway data.
	UpdateMovementSwayData();
	UpdateAimSwayData();

	// Update offset data.
	UpdateFallingOffsetData();
}

void UFirstPersonCharacterAnimInstance::UpdateAimData(float DeltaSeconds)
{
	const FRotator PreviousAimRotation = AimRotation;

	Super::UpdateAimData(DeltaSeconds);

	// Use a normalized delta to account for winding (e.g. 359.0 -> 1.0 should be 2.0, not -358.0).
	const FRotator RotationDelta = (bFirstUpdate ? FRotator::ZeroRotator :
									UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, PreviousAimRotation));

	/* Aim speed in degrees/second:
	 *		Degrees/1 Second = (Degrees/1 Frame) * (Frames/1 Second)
	 */
	AimSpeedRightLeft = FMath::Clamp((RotationDelta.Yaw * SafeInvertDeltaSeconds(DeltaSeconds)), -MAX_AIM_SPEED, MAX_AIM_SPEED);
	AimSpeedUpDown = FMath::Clamp((RotationDelta.Pitch * SafeInvertDeltaSeconds(DeltaSeconds)), -MAX_AIM_SPEED_UP_DOWN, MAX_AIM_SPEED_UP_DOWN);
}

void UFirstPersonCharacterAnimInstance::UpdateMovementSwayData()
{
	// Use the owning pawn's maximum movement speed as the bound for movement sway.
	const float MaxMovementSpeed = GetCrashCharacterMovementComponent()->GetMaxSpeed();

	// Calculate the forward/backward movement spring.
	float SpringTargetForwardBackward = UKismetMathLibrary::NormalizeToRange((LocalVelocity2D.X * MoveSwayForwardBackwardSpringModelData.InterpSpeed), 0.0f, MaxMovementSpeed);

	CurrentSpringMoveForwardBackward = UpdateFloatSpringInterp
	(
		CurrentSpringMoveForwardBackward,
		SpringTargetForwardBackward,
		SpringStateMoveForwardBackward,
		MoveSwayForwardBackwardSpringModelData
	);

	// Calculate the right/left movement spring.
	float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange((LocalVelocity2D.Y * MoveSwayRightLeftSpringModelData.InterpSpeed), 0.0f, MaxMovementSpeed);

	CurrentSpringMoveRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringMoveRightLeft,
		SpringTargetRightLeft,
		SpringStateMoveRightLeft,
		MoveSwayRightLeftSpringModelData
	);
}

void UFirstPersonCharacterAnimInstance::UpdateAimSwayData()
{
	// Calculate the right/left aim sway spring.
	const float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange((AimSpeedRightLeft * AimSwayRightLeftSpringModelData.InterpSpeed), 0.0f, MAX_AIM_SPEED);

	CurrentSpringAimRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringAimRightLeft,
		SpringTargetRightLeft,
		SpringStateAimRightLeft,
		AimSwayRightLeftSpringModelData
	);

	// Calculate the up/down aim sway spring.
	const float SpringTargetUpDown = UKismetMathLibrary::NormalizeToRange((AimSpeedUpDown * AimSwayUpDownSpringModelData.InterpSpeed), 0.0f, MAX_AIM_SPEED_UP_DOWN);

	CurrentSpringAimUpDown = UpdateFloatSpringInterp
	(
		CurrentSpringAimUpDown,
		SpringTargetUpDown,
		SpringStateAimUpDown,
		AimSwayUpDownSpringModelData
	);
}

void UFirstPersonCharacterAnimInstance::UpdateFallingOffsetData()
{
	// Use the owning pawn's jump velocity as the bound for the falling offset.
	const float MaxVerticalSpeed = GetCrashCharacterMovementComponent()->JumpZVelocity;

	// Calculate the falling offset spring.
	float SpringTargetFalling = UKismetMathLibrary::NormalizeToRange(LocalVelocity.Z, 0.0f, MaxVerticalSpeed);

	CurrentSpringFalling = UpdateFloatSpringInterp
	(
		CurrentSpringFalling,
		SpringTargetFalling,
		SpringStateFalling,
		FallingOffsetSpringModelData
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
	 * more intuitive range of (0 -> 100) when adjusting spring model data. */
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
