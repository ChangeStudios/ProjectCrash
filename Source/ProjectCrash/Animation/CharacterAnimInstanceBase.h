// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstanceBase.generated.h"

/**
 * Base animation instance for character animation blueprints. Collects relevant character data used for animation
 * blueprints.
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

private:

	/** Updates data about this character's current transform. */
	void UpdateTransformData();

	/** Updates data about this character's current velocity. */
	void UpdateVelocityData();

	/** Updates data about the owning player's aim. */
	void UpdateAimData();

	/** Updates data about this character's current movement states. */
	void UpdateCharacterStateData();

	/** Updates data used to blend this character's animations. */
	void UpdateBlendData();



	// Animation data.

// Transform data.
protected:

	/** Current location of this character, in world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	FVector WorldLocation;

	/** Current rotation of this character, in world space. Usually the same as the owning controller's rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	FRotator WorldRotation;

	/** The rate at which this character is turning. Usually the same as the X value of AimVelocity, since characters
	 * usually turn with their owning controller's rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Transform Data")
	float YawDeltaSpeed;

// Velocity data.
protected:

	/** The signed vector length of this character's current velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float SignedSpeed;

	/** This character's current velocity, relative to world space. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	FVector WorldVelocity;

	/** This character's current signed forward/backward speed, relative to its current rotation (i.e. local space). */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float ForwardBackwardSpeed;

	/** This character's current signed right/left speed, relative to its current rotation (i.e. local space). */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	float RightLeftSpeed;

	/** Whether this character's absolute speed is greater than 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Velocity Data")
	bool bHasVelocity;

// Aim data.

	/** The controller's current aim rotation pitch. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float AimPitch;

	/** The controller's current aim rotation yaw. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float AimYaw;

	/** The rate at which the controller is changing its pitch, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float UpDownAimSpeed;

	/** The rate at which the controller is changing its yaw, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float RightLeftAimSpeed;

// Character state data.

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

	/** The weight with which additive upper-body animations will be blended. Used to smoothly blend additives. Set to
	 * 1.0 while montages are playing, and smoothly lerped back to 0.0 when they stop. */
	UPROPERTY(BlueprintReadOnly, Category = "Blend Data")
	float UpperBodyAdditiveWeight;
};
