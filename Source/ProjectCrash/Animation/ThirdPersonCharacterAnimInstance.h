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

	// Construction.

public:

	/** Default constructor. */
	UThirdPersonCharacterAnimInstance();



	// Animation updates.

public:

	/** Updates third-person debug data in the editor. */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	/** Updates this character's current distance from the ground. */
	virtual void UpdateTransformData(float DeltaSeconds) override;

	/** Collects additional character velocity data used to drive third-person animations. */
	virtual void UpdateVelocityData(float DeltaSeconds) override;

	/** Initializes the land recovery animation alpha when landing. */
	virtual void UpdateCharacterStateData(float DeltaSeconds) override;



	// Animation data.

// Transform data.
protected:

	/** This character's current distance above the ground beneath them. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float GroundDistance;

	/** Change in GroundDistance this update. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float GroundDistanceDelta;

// Velocity data.
protected:

	/** This character's local velocity at the previous update. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LastLocalVelocity;

	/** This character's acceleration this frame, in cm/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector Acceleration;

	/** The cardinal direction of this character current local velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	EAnimCardinalDirection LocalVelocityDirection;

// Blend data.
protected:

	/** Alpha used to blend the additive recovery animation when landing. When hitting the ground, this is set to a
	 * value between 0.0 and 1.0 to blend the recovery animation, depending on how hard this character landed. */
	UPROPERTY(BlueprintReadOnly, Category = "Blend Data")
	float LandRecoveryAlpha;



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



	// Debugging.

#if WITH_EDITORONLY_DATA

protected:

	/** Updates this animation instance's preview actor's data with debug properties. */
	void UpdateDebugData(float DeltaSeconds);

	/** Set to simulate the preview character instance's world velocity. Note that this does not actually change the
	 * preview actor's velocity, so some features, like orientation warping, won't be simulated properly. */
	UPROPERTY(EditDefaultsOnly, Category = "Debugging", DisplayName = "Simulated Velocity", Meta = (UIMin = "-600", UIMax = "600"))
	FVector Debug_WorldVelocity;

	/** Set to simulate the preview character instance's aim pitch. */
	UPROPERTY(EditDefaultsOnly, Category = "Debugging", DisplayName = "Simulated Pitch", Meta = (UIMin = "-90", UIMax = "90"))
	float Debug_Pitch;

#endif // WITH_EDITORONLY_DATA
};
