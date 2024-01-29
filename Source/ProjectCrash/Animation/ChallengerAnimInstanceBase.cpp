// Copyright Samuel Reitich 2024.


#include "Animation/ChallengerAnimInstanceBase.h"

void UChallengerAnimInstanceBase::UpdateAnimData(UCharacterAnimData* NewAnimData)
{
	if (NewAnimData)
	{
		CurrentAnimationData = NewAnimData;
	}
}
