// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstanceBase.generated.h"

class UCrashCharacterMovementComponent;

/**
 * Base animation instance for character animation blueprints. Collects relevant character data used for animation
 * blueprints. The owning character must have a movement component of type CrashCharacterMovementComponent.
 *
 * Technically, this animation instance can be used on non-characters, but some of its data will not be valid,
 * especially if not used on a pawn.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCharacterAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCharacterAnimInstanceBase();



	// Animation updates.

public:

	/** Updates this character's animation data each frame. */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	/** Updates data about this character's current location and rotation. */
	virtual void UpdateTransformData(float DeltaSeconds);

	/** Updates data about this character's current velocity. */
	virtual void UpdateVelocityData(float DeltaSeconds);

	/** Updates data about the owning player's aim. */
	virtual void UpdateAimData(float DeltaSeconds);

	/** Updates data about this character's current movement states. */
	virtual void UpdateCharacterStateData(float DeltaSeconds);

	/** Updates data used to blend this character's animations. */
	virtual void UpdateBlendData(float DeltaSeconds);

	/** Whether this is our first animation update. Used to avoid initializing deltas to high values (e.g., we
	 * initialize AimRotation to FRotator(0.0), but the player's aim could be (0.0f, 90.0f, 0.0f), resulting in a high
	 * RotationDelta value on the first frame this is updated). */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bFirstUpdate;



	// Animation data.

// Transform data.
protected:

	/** Current location of this character, in world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	FVector WorldLocation;

	/** Current rotation of this character, in world space. Usually the same as the owning controller's rotation,
	 * excluding pitch. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	FRotator WorldRotation;

	/** The rate at which this character is turning. Usually the same as the X value of AimVelocity, since characters
	 * often turn with their owning controller's rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	float YawDeltaSpeed;

// Velocity data.
protected:

	/** This character's current velocity, relative to world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector WorldVelocity;

	/** This character's current velocity, relative to its world rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity;

	/** This character's current local velocity with vertical velocity (Z) masked out. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity2D;

	/** This character's current local velocity, normalized to its maximum movement speed. Vertical velocity (Z) is
	 * masked out. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector LocalVelocity2DNormalized;

	/** The direction, in local space, in which this character is moving. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float LocalVelocityDirectionAngle;

	/** Whether this character's absolute speed is greater than 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	bool bHasVelocity;

// Aim data.
protected:

	/** This character's current base aim rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	FRotator AimRotation;

// Character state data.
protected:

	/** Whether this character is currently on the ground. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	bool bIsOnGround;

	/** Whether this character's vertical velocity is greater than 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	bool bIsJumping;

	/** Whether this character's vertical velocity is less than 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	bool bIsFalling;

	/** The time until this character, if jumping, reaches the height of their jump. */
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float TimeToJumpApex;

// Blend data.
protected:

	/** The weight with which additive upper-body animations will be blended. Used to smoothly blend additives. Set to
	 * 1.0 while montages are playing, and smoothly lerped back to 0.0 when they stop. */
	UPROPERTY(BlueprintReadOnly, Category = "Blend Data")
	float UpperBodyAdditiveWeight;



	// Utils.

protected:

	/** Returns the inverse (frames/second) of DeltaSeconds, making sure not to divide by 0. Returns 0 if DeltaSeconds
	 * is 0. */
	FORCEINLINE static float SafeInvertDeltaSeconds(float DeltaSeconds) { return (DeltaSeconds > 0.0f ? (1.0f / DeltaSeconds) : (0.0f)); }

	/** Returns this animation instance's owning pawn's movement component has a CrashCharacterMovementComponent. */
	UCrashCharacterMovementComponent* GetCrashCharacterMovementComponent() const;
};
