// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/CharacterAnimInstanceBase.h"
#include "ThirdPersonCharacterAnimInstance.generated.h"

/**
 * Represents one of four cardinal directions relative to the character's rotation. Used for selecting 4D locomotion
 * animations.
 */
UENUM(BlueprintType)
enum class EAnimCardinalDirection : uint8
{
	Forward,
	Backward,
	Right,
	Left
};



/**
 * Base animation instance for third-person animation blueprints. Collects additional data used for animating
 * characters in third-person.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UThirdPersonCharacterAnimInstance : public UCharacterAnimInstanceBase
{
	GENERATED_BODY()

	// Animation updates.

protected:

	/** Collects additional character velocity data used to drive third-person animations. */
	virtual void UpdateVelocityData(float DeltaSeconds) override;



	// Animation data.

// Velocity data.
protected:

	/** The cardinal direction of this character current local velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	EAnimCardinalDirection LocalVelocityDirection;



	// Utils.

public:

	/**
	 * Converts a given direction, represented by an angle, into a cardinal direction.
	 *
	 * @param LocalAngle				The direction which will be converted to a cardinal direction.
	 * @param DeadZone					Leniency for "Forward" and "Backward" directions. Used to increase the range in
	 *									which "Forward" or "Backward" will be selected, for when these directions should
	 *									be favored.
	 * @param bUseCurrentDirection		If true, the given DeadZone will be increased if already moving forward or
	 *									backward, making it more difficult to change directions and preventing rapid
	 *									changes.
	 * @param CurrentDirection			The current cardinal direction, used for skewing the dead-zone if
	 *									bUseCurrentDirection is true.
	 * @return							The given direction as a cardinal direction, taking into account any given
	 *									dead-zones.
	 */
	static EAnimCardinalDirection SelectCardinalDirectionFromAngle(float LocalAngle, float DeadZone, bool bUseCurrentDirection, EAnimCardinalDirection CurrentDirection);
};
