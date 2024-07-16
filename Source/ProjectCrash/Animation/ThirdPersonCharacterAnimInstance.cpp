// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ThirdPersonCharacterAnimInstance.h"

/** The dead-zone value used for converting this character's local velocity into a cardinal direction. */
#define CARDINAL_DEAD_ZONE 10.0f



void UThirdPersonCharacterAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
	Super::UpdateVelocityData(DeltaSeconds);

	// Update which local cardinal direction this character is currently moving.
	LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle, CARDINAL_DEAD_ZONE, true, LocalVelocityDirection);
}

EAnimCardinalDirection UThirdPersonCharacterAnimInstance::SelectCardinalDirectionFromAngle(float LocalAngle, float DeadZone, bool bUseCurrentDirection, EAnimCardinalDirection CurrentDirection)
{
	const float AbsLocalAngle = FMath::Abs(LocalAngle);
	float ForwardDeadZone = DeadZone;
	float BackwardDeadZone = DeadZone;

	/* Increase the dead-zone of the current direction, if desired. This makes it more difficult to change from the
	 * current direction, so we don't rapidly switch between directions. */
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == EAnimCardinalDirection::Forward)
		{
			ForwardDeadZone *= 2.0f;
		}
		else if (CurrentDirection == EAnimCardinalDirection::Backward)
		{
			BackwardDeadZone *= 2.0f;
		}
	}

	// Forward range.
	if (AbsLocalAngle <= (45.0f + ForwardDeadZone))
	{
		return EAnimCardinalDirection::Forward;
	}
	// Backward range.
	else if (AbsLocalAngle >= (135.0f - BackwardDeadZone))
	{
		return EAnimCardinalDirection::Backward;
	}
	// Right range.
	else if (LocalAngle > 0.0f)
	{
		return EAnimCardinalDirection::Right;
	}
	// Left range.
	else
	{
		return EAnimCardinalDirection::Left;
	}
}

