// Copyright Samuel Reitich. All rights reserved.


#include "Animation/CharacterAnimInstanceBase.h"


UCharacterAnimInstanceBase::UCharacterAnimInstanceBase()
{
	// Enable multithreading for animation updates.
	bUseMultiThreadedAnimationUpdate = true;
}

void UCharacterAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// Update animation data.

	UpdateTransformData();

	UpdateVelocityData();

	UpdateAimData();

	UpdateCharacterStateData();

	UpdateBlendData();
}

void UCharacterAnimInstanceBase::UpdateTransformData()
{
}

void UCharacterAnimInstanceBase::UpdateVelocityData()
{
}

void UCharacterAnimInstanceBase::UpdateAimData()
{
}

void UCharacterAnimInstanceBase::UpdateCharacterStateData()
{
}

void UCharacterAnimInstanceBase::UpdateBlendData()
{
}
