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
}

void UChallengerAnimInstanceBase_FPP::CalculateMovementSway()
{
}

void UChallengerAnimInstanceBase_FPP::CalculateAimSway()
{
}
