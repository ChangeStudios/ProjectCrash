// Copyright Samuel Reitich. All rights reserved.


#include "Animation/MyFirstPersonCharacterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

// Inverse of the minimum frame rate required to perform spring calculations.
#define MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS 0.1f // 10 fps
/* Universal multiplier applied to spring model stiffness. Used to scale stiffness values to a more intuitive
 * range for designers. */
#define SPRING_STIFFNESS_SCALER 35.0f

UMyFirstPersonCharacterAnimInstance::UMyFirstPersonCharacterAnimInstance():
	AimSpeedRightLeft(0),
	AimSpeedUpDown(0),
	CurrentSpringMoveForwardBackward(0),
	CurrentSpringMoveRightLeft(0),
	CurrentSpringAimRightLeft(0),
	CurrentSpringAimUpDown(0),
	CurrentSpringFalling(0)
{
	bUseMultiThreadedAnimationUpdate = true;
}

void UMyFirstPersonCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner)
	{
		return;
	}

	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());
	if (!CharMovementComp || (CharMovementComp->MovementMode == MOVE_None))
	{
		return;
	}

	UpdateVelocityData();

	UpdateAimData(DeltaSeconds);

	UpdateMovementSwayData();

	UpdateAimSwayData();
}

void UMyFirstPersonCharacterAnimInstance::UpdateVelocityData()
{
	APawn* PawnOwner = TryGetPawnOwner();
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());

	const FVector WorldVelocity = PawnOwner->GetVelocity();
	const FRotator WorldRotation = PawnOwner->GetActorRotation();

	// Local velocity.
	LocalVelocity = WorldRotation.UnrotateVector(WorldVelocity);
	LocalVelocity2D = LocalVelocity * FVector(1.0f, 1.0f, 0.0f);

	// Normalized local velocity.
	const float MaxMovementSpeed = CharMovementComp->GetMaxSpeed();
	const float NormalizedX = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.X, 0.0f, MaxMovementSpeed), -1.0f, 1.0f);
	const float NormalizedY = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.Y, 0.0f, MaxMovementSpeed), -1.0f, 1.0f);
	LocalVelocity2DNormalized = FVector(NormalizedX, NormalizedY, 0.0f);
}

void UMyFirstPersonCharacterAnimInstance::UpdateAimData(float DeltaSeconds)
{
	const FRotator PreviousAimRotation = AimRotation;

	AimRotation = TryGetPawnOwner()->GetBaseAimRotation();
	AimRotation.Pitch = FRotator::NormalizeAxis(AimRotation.Pitch); // Fix for a problem with how UE replicates aim.

	// Use a normalized delta to account for winding (e.g. 359.0 -> 1.0 should be 2.0, not -358.0).
	const FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, PreviousAimRotation);

	const float InverseDeltaSeconds = ((DeltaSeconds > 0.0f) ? (1.0f / DeltaSeconds) : 0.0f); // Avoid dividing by 0.

	/* Aim speed in degrees/second:
	 *		Degrees/1 Second = (Degrees/1 Frame) * (Frames/1 Second)
	 */
	AimSpeedRightLeft = RotationDelta.Yaw * InverseDeltaSeconds;
	AimSpeedUpDown = RotationDelta.Pitch * InverseDeltaSeconds;
}

void UMyFirstPersonCharacterAnimInstance::UpdateMovementSwayData()
{
	// Use the owning pawn's maximum movement speed as the bound for movement sway.
	const float MaxMovementSpeed = TryGetPawnOwner()->GetMovementComponent()->GetMaxSpeed();

	// Calculate the forward/backward movement spring.
	const float ClampedSpeedX = FMath::Clamp(LocalVelocity2D.X, -MaxMovementSpeed, MaxMovementSpeed);
	const float SpringTargetForwardBackward = UKismetMathLibrary::NormalizeToRange((ClampedSpeedX * MoveSwayForwardBackwardSpringModelData.InterpSpeed), 0.0f, MaxMovementSpeed);

	CurrentSpringMoveForwardBackward = UpdateFloatSpringInterp
	(
		CurrentSpringMoveForwardBackward,
		SpringTargetForwardBackward,
		SpringStateMoveForwardBackward,
		MoveSwayForwardBackwardSpringModelData
	);

	// Calculate the right/left movement spring.
	const float ClampedSpeedY = FMath::Clamp(LocalVelocity2D.Y, -MaxMovementSpeed, MaxMovementSpeed);
	const float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange((ClampedSpeedY * MoveSwayRightLeftSpringModelData.InterpSpeed), 0.0f, MaxMovementSpeed);

	CurrentSpringMoveRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringMoveRightLeft,
		SpringTargetRightLeft,
		SpringStateMoveRightLeft,
		MoveSwayRightLeftSpringModelData
	);
}

void UMyFirstPersonCharacterAnimInstance::UpdateAimSwayData()
{
	const float MaxAimSpeed = 720.0f;
	const float MaxAimSpeedUpDown = (MaxAimSpeed / 2.0f); // Halved because characters' pitch has half the range of their yaw: (-90 -> 90) vs. (0 -> 360).

	// Calculate the right/left aim sway spring.
	const float ClampedSpeedRightLeft = FMath::Clamp(AimSpeedRightLeft, -MaxAimSpeed, MaxAimSpeed);
	const float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange((ClampedSpeedRightLeft * AimSwayRightLeftSpringModelData.InterpSpeed), 0.0f, MaxAimSpeed);

	CurrentSpringAimRightLeft = UpdateFloatSpringInterp
	(
		CurrentSpringAimRightLeft,
		SpringTargetRightLeft,
		SpringStateAimRightLeft,
		AimSwayRightLeftSpringModelData
	);

	// Calculate the up/down aim sway spring.
	const float ClampedSpeedUpDown = FMath::Clamp(AimSpeedUpDown, -MaxAimSpeedUpDown, MaxAimSpeedUpDown);
	const float SpringTargetUpDown = UKismetMathLibrary::NormalizeToRange((ClampedSpeedUpDown * AimSwayUpDownSpringModelData.InterpSpeed), 0.0f, MaxAimSpeedUpDown);

	CurrentSpringAimUpDown = UpdateFloatSpringInterp
	(
		CurrentSpringAimUpDown,
		SpringTargetUpDown,
		SpringStateAimUpDown,
		AimSwayUpDownSpringModelData
	);
}

void UMyFirstPersonCharacterAnimInstance::UpdateFallingOffsetData()
{
}

float UMyFirstPersonCharacterAnimInstance::UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FMyFloatSpringModelData& SpringData)
{
	const float DeltaSeconds = GetDeltaSeconds();

	// Don't bother performing spring calculations at low frame-rates.
	if (DeltaSeconds > MIN_DELTA_TIME_FOR_SPRING_CALCULATIONS)
	{
		return SpringCurrent;
	}

	/* Apply an arbitrary multiplier to the spring's stiffness value. This scales viable spring stiffness values to a
	 * more intuitive range of (0 -> 100) when adjusting spring model data. */
	const float EffectiveStiffness = SpringData.Stiffness * SPRING_STIFFNESS_SCALER;

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
