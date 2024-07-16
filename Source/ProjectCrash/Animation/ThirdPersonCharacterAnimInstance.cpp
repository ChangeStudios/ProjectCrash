// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ThirdPersonCharacterAnimInstance.h"

void UThirdPersonCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void UThirdPersonCharacterAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
	Super::UpdateVelocityData(DeltaSeconds);

	// Determine which local cardinal direction this character is currently moving.
	LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle, 0.0f, true, LocalVelocityDirection);
}

EAnimCardinalDirection UThirdPersonCharacterAnimInstance::SelectCardinalDirectionFromAngle(float LocalAngle, float DeadZone, bool bUseCurrentDirection, EAnimCardinalDirection CurrentDirection)
{
	return EAnimCardinalDirection::Forward;
}

