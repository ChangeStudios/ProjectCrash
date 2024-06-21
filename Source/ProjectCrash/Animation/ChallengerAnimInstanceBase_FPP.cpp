// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ChallengerAnimInstanceBase_FPP.h"

#include "Camera/CameraComponent.h"
#include "Characters/ChallengerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UChallengerAnimInstanceBase_FPP::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// Only update first-person animations on the local client. Updates can be turned back on when spectating.
	if (OwningChallenger && !OwningChallenger->IsLocallyControlled())
	{
		bUseMultiThreadedAnimationUpdate = false;
	}
}

float UChallengerAnimInstanceBase_FPP::UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FFloatSpringInterpData& SpringData)
{
	const float DeltaSeconds = GetDeltaSeconds();

	// Don't perform the spring calculation at low frame-rates. I've set 10 FPS arbitrarily.
	if (DeltaSeconds > 0.1f)
	{
		return SpringCurrent;
	}

	/* The stiffness value given to the spring model requires extremely high numbers. We scale it here to make
	 * adjusting the spring data more intuitive. 35.0 is a magic number chosen based on testing. It effectively scales
	 * the stiffness to a range of 0-100. */
	const float EffectiveStiffness = SpringData.Stiffness * 35.0f;

	// Perform the float spring interpolation with the given data.
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

void UChallengerAnimInstanceBase_FPP::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateAimOffset();

	UpdateAimSpeed();

	CalculateMovementSway();

	CalculateAimSway();
}

void UChallengerAnimInstanceBase_FPP::UpdateAimOffset()
{
	// Cache the character's current normalized camera pitch.
	if (IsValid(OwningChallenger))
	{
		FVector CurrentRotAsVector = OwningChallenger->GetFirstPersonCameraComponent()->GetRelativeRotation().Vector();
		CurrentRotAsVector.Normalize();
		NormalizedCameraPitch = CurrentRotAsVector.Z;
	}
}

void UChallengerAnimInstanceBase_FPP::UpdateAimSpeed()
{
	if (IsValid(OwningChallenger))
	{
		// Update aim rotation.
		PawnRotation = OwningChallenger->GetActorRotation();
		PreviousAimRotation = CurrentAimRotation;
		CurrentAimRotation = OwningChallenger->GetFirstPersonCameraComponent()->GetComponentRotation();
		const FRotator RotationSinceLastUpdate = CurrentAimRotation - PreviousAimRotation;

		// Retrieve time differences.
		const float DeltaSeconds = GetDeltaSeconds();
		const float TimeSinceLastUpdate = DeltaSeconds > 0.0f ? DeltaSeconds : 1.0f;

		// Calculate rotation speed.
		const FRotator RotationSpeed = RotationSinceLastUpdate * (1.0f / TimeSinceLastUpdate);

		/** Clamp the maximum aim rotation speed to the upper and lower values used to normalize the speed when using
		 * it to apply additive aim sway animations. */
		AimUpDownSpeed = FMath::Clamp(RotationSpeed.Pitch, -CurrentAnimationData->MaxAimSpeed, CurrentAnimationData->MaxAimSpeed);
		AimRightLeftSpeed = FMath::Clamp(RotationSpeed.Yaw, -CurrentAnimationData->MaxAimSpeed, CurrentAnimationData->MaxAimSpeed);
	}
}

void UChallengerAnimInstanceBase_FPP::CalculateMovementSway()
{
	if (IsValid(OwningChallenger))
	{
		// Calculate target spring values using current movement speeds.
		const float MaxWalkSpeed = OwningChallenger->GetCharacterMovement()->MaxWalkSpeed;
		const float SpringTargetForwardBackward = UKismetMathLibrary::NormalizeToRange(ForwardBackwardSpeed, 0.0f, MaxWalkSpeed) * CurrentAnimationData->MoveSwayForwardBackwardSpringData.InterpSpeed;
		const float SpringTargetRightLeft = UKismetMathLibrary::NormalizeToRange(RightLeftSpeed, 0.0, MaxWalkSpeed) * CurrentAnimationData->MoveSwayRightLeftSpringData.InterpSpeed;

		// Update the current spring value for forward/backward movement sway.
		CurrentSpringMoveForwardBackward = UpdateFloatSpringInterp
		(
			CurrentSpringMoveForwardBackward,
			SpringTargetForwardBackward,
			SpringStateMoveForwardBackward,
			CurrentAnimationData->MoveSwayForwardBackwardSpringData
		);

		// Update the current spring value for right/left movement sway.
		CurrentSpringMoveRightLeft = UpdateFloatSpringInterp
		(
			CurrentSpringMoveRightLeft,
			SpringTargetRightLeft,
			SpringStateMoveRightLeft,
			CurrentAnimationData->MoveSwayRightLeftSpringData
		);
	}
}

void UChallengerAnimInstanceBase_FPP::CalculateAimSway()
{
	if (IsValid(OwningChallenger))
	{
		// Normalizes a value between -1.0 and 1.0 instead of 0.0 and 1.0.
		auto NormalizeNegative = [](const float& X, const float& Min, const float& Max)
		{
			return ( ( (X - Min) / (Max - Min) ) * 2.0f ) - 1.0f;
		};
		
		// Calculate target spring values using current aim speeds.
		const float SpringTargetUpDown = NormalizeNegative(AimUpDownSpeed, -CurrentAnimationData->MaxAimSpeed, CurrentAnimationData->MaxAimSpeed) * CurrentAnimationData->AimSwayUpDownSpringData.InterpSpeed;
		const float SpringTargetRightLeft = NormalizeNegative(AimRightLeftSpeed, -CurrentAnimationData->MaxAimSpeed, CurrentAnimationData->MaxAimSpeed) * CurrentAnimationData->AimSwayRightLeftSpringData.InterpSpeed;

		// Update the current spring value for up/down aim sway.
		CurrentSpringAimUpDown = UpdateFloatSpringInterp
		(
			CurrentSpringAimUpDown,
			SpringTargetUpDown,
			SpringStateAimUpDown,
			CurrentAnimationData->AimSwayUpDownSpringData
		);

		// Update the current spring value for right/left aim sway.
		CurrentSpringAimRightLeft = UpdateFloatSpringInterp
		(
			CurrentSpringAimRightLeft,
			SpringTargetRightLeft,
			SpringStateAimRightLeft,
			CurrentAnimationData->AimSwayRightLeftSpringData
		);
	}
}
