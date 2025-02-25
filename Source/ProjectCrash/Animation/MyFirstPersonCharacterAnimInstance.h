// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyFirstPersonCharacterAnimInstance.generated.h"

/**
 * Defines the behavior of a spring model. Used for performing calculations for sway animations.
 */
USTRUCT(BlueprintType)
struct FFloatSpringModelData
{
	GENERATED_BODY()

	/* Controls the amplitude of the spring model. This value is signed, so setting it to a negative number reverses the
	 * direction of the spring (e.g. to create the effect of leading versus lagging). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = -10.0f, UIMin = -10.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float InterpSpeed = 1.0f;

	// Represents the stiffness of this spring. Higher values reduce overall oscillation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Stiffness = 25.0f;

	/* The amount of damping applied to the spring. 0.0 means no damping (full oscillation), 1.0 means full damping
	 * (no oscillation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float CriticalDampingFactor = 0.5f;

	// A multiplier that simulates the spring's, affecting the amount of force required to oscillate it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Mass = 10.0f;
};

UCLASS(Abstract)
class PROJECTCRASH_API UMyFirstPersonCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	// Construction.

public:

	UMyFirstPersonCharacterAnimInstance();



	// Animation updates.

public:

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	// Calculate velocity data this frame.
	void UpdateVelocityData();

	// Calculate aim data this frame.
	void UpdateAimData(float DeltaSeconds);

	// Updates movement sway data using a spring model.
	void UpdateMovementSwayData();

	void UpdateAimSwayData();

	void UpdateFallingOffsetData();



	// Velocity.

protected:

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



	// Aim.

protected:

	/** This character's current base aim rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	FRotator AimRotation;

	/** The rate at which the owning character's aim yaw is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Right/Left)")
	float AimSpeedRightLeft;

	/** The rate at which the owning character's aim pitch is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Up/Down)")
	float AimSpeedUpDown;


	// Movement sway.

protected:

	// The spring model used to drive forward/backward movement sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Forward/Backward Sway Spring Model")
	FFloatSpringModelData MoveSwayForwardBackwardSpringModelData;

	// The spring model used to drive right/left movement sway for this animation instance.
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Right/Left Sway Spring Model")
	FFloatSpringModelData MoveSwayRightLeftSpringModelData;

protected:

	// The current spring value of the forward/backward movement sway spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Forward/Backward)")
	float CurrentSpringMoveForwardBackward;

	// The current spring value of the right/left movement sway spring.
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Right/Left)")
	float CurrentSpringMoveRightLeft;

private:

	// Spring state for the forward/backward movement sway's spring calculations.
	FFloatSpringState SpringStateMoveForwardBackward;

	// Spring state for the right/left movement sway's spring calculations.
	FFloatSpringState SpringStateMoveRightLeft;



	// Aim sway.

protected:

	/** The current spring value for the right/left aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway", DisplayName = "Current Aim Sway Value (Right/Left)")
	float CurrentSpringAimRightLeft;

	/** The current spring value for the up/down aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway", DisplayName = "Current Aim Sway Value (Up/Down)")
	float CurrentSpringAimUpDown;

private:

	/** Spring state for the right/left aim sway's spring calculations. */
	FFloatSpringState SpringStateAimRightLeft;

	/** Spring state for the up/down aim sway's spring calculations. */
	FFloatSpringState SpringStateAimUpDown;



	// Falling offset.

protected:

	/** The current spring value of the falling offset spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Falling Offset", DisplayName = "Current Falling Offset Value")
	float CurrentSpringFalling;

private:

	/** Spring state for the falling offset's spring calculations. */
	FFloatSpringState SpringStateFalling;


	// Utils.

protected:

	/** Returns the inverse (frames/second) of DeltaSeconds, making sure not to divide by 0. Returns 0 if DeltaSeconds
	 * is 0. */
	FORCEINLINE static float SafeInvertDeltaSeconds(float DeltaSeconds) { return (DeltaSeconds > 0.0f ? (1.0f / DeltaSeconds) : (0.0f)); }

	/**
	 * Performs a float spring interpolation using the given data.
	 *
	 * @param SpringCurrent			The current spring interpolation value.
	 * @param SpringTarget			The target spring interpolation value.
	 * @param SpringState			Data for the calculating spring model. Each spring model should use a unique spring
	 *								state variable.
	 * @param SpringData			Data used to define the behavior of the spring model.
	 * @return						The resulting spring interpolation value.
	 */
	float UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FFloatSpringModelData& SpringData);
};
