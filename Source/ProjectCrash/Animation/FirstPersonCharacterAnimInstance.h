// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/CharacterAnimInstanceBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "FirstPersonCharacterAnimInstance.generated.h"

/**
 * Defines the behavior of a spring model. Used for performing calculations for sway animations.
 */
USTRUCT(BlueprintType)
struct FFloatSpringModelData
{
	GENERATED_BODY()

	/** Scalar used to scale the amplitude of the spring model and control the strength of its effect. Setting this to
	 * a negative value can create the appearance of "leading" a sway, if that's desired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = -10.0f, UIMin = -10.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float InterpSpeed = 1.0f;

	/** Represents the stiffness of this spring. Higher values reduce overall oscillation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Stiffness = 25.0f;

	/** The amount of damping applied to the spring. 0.0 means no damping (full oscillation), 1.0 means full damping
	 * (no oscillation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float CriticalDampingFactor = 0.5f;

	/** A multiplier that behaves like mass on the spring, affecting the amount of force required to change its
	 * position. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 100.0f, UIMax = 100.0f))
	float Mass = 10.0f;
};



/**
 * Base animation instance for first-person character animation blueprints. Performs calculations to determine movement
 * sway and aim sway values that would be tedious to do in blueprints.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UFirstPersonCharacterAnimInstance : public UCharacterAnimInstanceBase
{
	GENERATED_BODY()

	// Animation updates.

public:

	/** Performs sway calculations each animation update. */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	/** Collects additional aim data used to drive aim sway. */
	virtual void UpdateAimData(float DeltaSeconds) override;

	/** Updates movement sway data using a spring model. */
	void UpdateMovementSwayData();

	/** Updates aim sway data using a spring model. */
	void UpdateAimSwayData();



	// Properties.

protected:

	/** The spring model used to drive forward/backward movement sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Sway Data|Movement Sway")
	FFloatSpringModelData MovementSwayForwardBackwardData;

	/** The spring model used to drive right/left movement sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Sway Data|Movement Sway")
	FFloatSpringModelData MovementSwayRightLeftData;

	/** The spring model used to drive right/left aim sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Sway Data|Aim Sway")
	FFloatSpringModelData AimSwayRightLeftData;

	/** The spring model used to drive up/down aim sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Sway Data|Aim Sway")
	FFloatSpringModelData AimSwayUpDownData;



	// Animation data.

// Aim data.
protected:

	/** The rate at which the owning character's aim yaw is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float AimSpeedRightLeft;

	/** The rate at which the owning character's aim pitch is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data")
	float AimSpeedUpDown;

	/* The maximum absolute speed, in degrees/second, with which aim sway will be calculated. This acts as a bound for
	 * scaling sway with speed, preventing issues like arms completely flying away when turning too quickly. */
	const float MaxAimSpeed = 360.0f;



	// Movement sway.

protected:

	/** The current spring value of the forward/backward movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway")
	float CurrentSpringMoveForwardBackward;

	/** The current spring value of the right/left movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway")
	float CurrentSpringMoveRightLeft;

private:

	/** Spring state for the forward/backward movement sway's spring calculations. */
	FFloatSpringState SpringStateMoveForwardBackward;

	/** Spring state for the right/left movement sway's spring calculations. */
	FFloatSpringState SpringStateMoveRightLeft;



	// Aim sway.

protected:

	/** The current spring value for the right/left aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway")
	float CurrentSpringAimRightLeft;

	/** The current spring value for the up/down aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Aim Sway")
	float CurrentSpringAimUpDown;

private:

	/** Spring state for the right/left aim sway's spring calculations. */
	FFloatSpringState SpringStateAimRightLeft;

	/** Spring state for the up/down aim sway's spring calculations. */
	FFloatSpringState SpringStateAimUpDown;



	// Utils.

private:

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
