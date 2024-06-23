// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ChallengerAnimInstanceBase_TPP.h"

#include "AnimData/CharacterAnimData.h"
#include "Characters/CrashCharacter.h"

void UChallengerAnimInstanceBase_TPP::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateAimOffset();
}

void UChallengerAnimInstanceBase_TPP::UpdateAimOffset()
{
	// Cache the character's current normalized aim pitch.
	if (OwningCharacter)
	{
		FVector CurrentRotAsVector = OwningCharacter->GetBaseAimRotation().Vector();
		CurrentRotAsVector.Normalize();
		NormalizedAimPitch = FMath::Clamp(CurrentRotAsVector.Z, CurrentAnimationData->MinNormalizedPitch, CurrentAnimationData->MaxNormalizedPitch);
	}
}