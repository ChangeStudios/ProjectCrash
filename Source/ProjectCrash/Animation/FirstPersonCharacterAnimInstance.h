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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model", Meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 100.0f, UIMax = 100.0f))
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

	// Construction.

public:

	/** Default constructor. */
	UFirstPersonCharacterAnimInstance();



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

	/** Updates falling offset data using a spring model. */
	void UpdateFallingOffsetData();



	// Properties.

protected:

	/** The spring model used to drive forward/backward movement sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Forward/Backward Sway Spring Model")
	FFloatSpringModelData MoveSwayForwardBackwardSpringModelData;

	/** The spring model used to drive right/left movement sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Movement Sway", DisplayName = "Right/Left Sway Spring Model")
	FFloatSpringModelData MoveSwayRightLeftSpringModelData;

	/** The spring model used to drive right/left aim sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Aim Sway", DisplayName = "Right/Left Sway Spring Model")
	FFloatSpringModelData AimSwayRightLeftSpringModelData;

	/** The spring model used to drive up/down aim sway for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Aim Sway", DisplayName = "Up/Down Sway Spring Model")
	FFloatSpringModelData AimSwayUpDownSpringModelData;

	/** The spring model used to drive the falling (vertical movement) offset for this animation instance. */
	UPROPERTY(EditDefaultsOnly, Category = "Spring Models|Falling Offset", DisplayName = "Falling Offset Spring Model")
	FFloatSpringModelData FallingOffsetSpringModelData;



	// Animation data.

// Aim data.
protected:

	/** The rate at which the owning character's aim yaw is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Right/Left)")
	float AimSpeedRightLeft;

	/** The rate at which the owning character's aim pitch is changing, in degrees/second. */
	UPROPERTY(BlueprintReadOnly, Category = "Aim Data", DisplayName = "Aim Speed (Up/Down)")
	float AimSpeedUpDown;



	// Movement sway.

protected:

	/** The current spring value of the forward/backward movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Forward/Backward)")
	float CurrentSpringMoveForwardBackward;

	/** The current spring value of the right/left movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Sway Data|Movement Sway", DisplayName = "Current Movement Sway Value (Right/Left)")
	float CurrentSpringMoveRightLeft;

private:

	/** Spring state for the forward/backward movement sway's spring calculations. */
	FFloatSpringState SpringStateMoveForwardBackward;

	/** Spring state for the right/left movement sway's spring calculations. */
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
