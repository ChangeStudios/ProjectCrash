// Copyright Samuel Reitich 2024.


#include "Animation/ChallengerAnimInstanceBase_FPP.h"

#include "Camera/CameraComponent.h"
#include "Characters/ChallengerBase.h"

void UChallengerAnimInstanceBase_FPP::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Enable multi-threading for updating animations.
	bUseMultiThreadedAnimationUpdate = true;
}

void UChallengerAnimInstanceBase_FPP::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// Cache the owning challenger character.
	OwningChallenger = TryGetPawnOwner() ? Cast<AChallengerBase>(TryGetPawnOwner()) : nullptr;
}

void UChallengerAnimInstanceBase_FPP::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateAimOffset();

	UpdateMovementVelocity();

	UpdateAimSpeed();

	CalculateMovementSway();

	CalculateAimSway();
}

void UChallengerAnimInstanceBase_FPP::UpdateAimOffset()
{
	// Cache the character's current normalized pitch.
	if (IsValid(OwningChallenger))
	{
		FVector CurrentRotAsVector = OwningChallenger->GetFirstPersonCameraComponent()->GetRelativeRotation().Vector();
		CurrentRotAsVector.Normalize();
		NormalizedCameraPitch = CurrentRotAsVector.Z;
	}
}

void UChallengerAnimInstanceBase_FPP::UpdateMovementVelocity()
{
	// Cache the character's current movement values.
	if (IsValid(OwningChallenger))
	{
		SignedSpeed = OwningChallenger->GetVelocity().Length();
	}
}

void UChallengerAnimInstanceBase_FPP::UpdateAimSpeed()
{
	if (IsValid(OwningChallenger))
	{
		PawnRotation = OwningChallenger->GetActorRotation();
		PreviousAimRotation = CurrentAimRotation;
		CurrentAimRotation = OwningChallenger->GetFirstPersonCameraComponent()->GetComponentRotation();

		const FRotator RotationSinceLastUpdate = CurrentAimRotation - PreviousAimRotation;

		const float DeltaSeconds = GetDeltaSeconds();
		const float TimeSinceLastUpdate = DeltaSeconds > 0.0f ? DeltaSeconds : 1.0f;

		const FRotator RotationSpeed = RotationSinceLastUpdate * (1.0f / TimeSinceLastUpdate);

		/** Clamp the maximum aim rotation speed to the upper and lower values used to normalize the speed when using
		 * it to apply additive aim sway animations. */
		AimUpDownSpeed = FMath::Clamp(RotationSpeed.Pitch, -MaxAimSpeed, MaxAimSpeed);
		AimRightLeftSpeed = FMath::Clamp(RotationSpeed.Yaw, -MaxAimSpeed, MaxAimSpeed);
	}
}

void UChallengerAnimInstanceBase_FPP::CalculateMovementSway()
{
}

void UChallengerAnimInstanceBase_FPP::CalculateAimSway()
{
}
