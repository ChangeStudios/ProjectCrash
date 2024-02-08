// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ChallengerAnimInstanceBase.h"
#include "AnimData/CharacterAnimData.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChallengerAnimInstanceBase_FPP.generated.h"

class UCrashAbilitySystemComponent;
class AChallengerBase;

/**
 * Base animation instance for Challenger characters' first-person animation blueprints. Only Challengers (i.e. player
 * characters) need first-person animation instances.
 */
UCLASS()
class PROJECTCRASH_API UChallengerAnimInstanceBase_FPP : public UChallengerAnimInstanceBase
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Default constructor. */
	virtual void NativeInitializeAnimation() override;

	/** Default initializer. Caches the animation's owning character. */
	virtual void NativeBeginPlay() override;



	// Utils.

protected:

	/** This animation instance's owning pawn, cached for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Crash|Animation")
	TObjectPtr<AChallengerBase> OwningChallenger;

	/** This animation instance's owning pawn's ASC, cached for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Crash|Animation")
	TObjectPtr<UCrashAbilitySystemComponent> OwningASC;

	/** Thread-safe function for checking if an ASC has a given tag. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Animation", meta = (BLueprintThreadSafe))
	bool ThreadSafeHasTagExact(UAbilitySystemComponent* ASC, FGameplayTag TagToSearch) const;

	/** Caches this animation's owning pawn's ASC after it is initialized. */
	UFUNCTION()
	void OnASCInitialized(UCrashAbilitySystemComponent* CrashASC);

	/**
	 * Performs a float spring interpolation using the given values.
	 *
	 * @param SpringCurrent			The current spring interpolation value.
	 * @param SpringTarget			The target spring interpolation value.
	 * @param SpringState			Data for the calculating spring model. Create a unique variable for each spring
	 *								model.
	 * @param SpringData			Data used for adjusting the spring model. Adjust this data to change the behavior
	 *								of the spring.
	 * @return						The resulting spring interpolation value.
	 */
	float UpdateFloatSpringInterp(float SpringCurrent, float SpringTarget, FFloatSpringState& SpringState, FFloatSpringInterpData& SpringData);



	// Animation updating.

public:

	/** Thread-safe animation update function. This is used instead of NativeUpdateAnimation to utilize multi-threading,
	 * avoiding a major bottleneck caused by traditional animation updating. This function can only call other thread-
	 * safe functions. */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	/** Update the character's base pose using its current camera pitch. */
	void UpdateAimOffset();

	/** Update the character movement data used for animation logic. */
	void UpdateMovementVelocity();

	/** Update the character and camera rotation data used for animation logic. */
	void UpdateAimSpeed();

	/** Calculates values used to apply additive movement sway. */
	void CalculateMovementSway();

	/** Calculates values used to apply additive aim sway. */
	void CalculateAimSway();



	// Character transform values updated each animation cycle to perform animation logic.

// Movement speed.
protected:

	/** The signed vector length of the pawn's current velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Character Transform")
	float SignedSpeed;

	/** The pawn's signed movement speed relative to their X-axis. */
	float ForwardBackwardSpeed;

	/** The pawn's signed movement speed relative to their Y-axis. */
	float RightLeftSpeed;

	/** The pawn's signed movement speed relative to their Z-axis. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Character Transform")
	float UpDownSpeed;

// Aim & rotation.
protected:

	/** Pawn's current world rotation. */
	FRotator PawnRotation;

	/** Pawn's camera's current world rotation. */
	FRotator CurrentAimRotation;

	/** Pawn's camera's world rotation at the last update. */
	FRotator PreviousAimRotation;

	/** Rate at which the pawn's X-rotation (yaw) changed this update, in degrees/second. */
	float AimRightLeftSpeed;

	/** Rate at which the pawn's Y-rotation (pitch) changed this update, in degrees/second. */
	float AimUpDownSpeed;

	/** The character's camera aim rotation pitch normalized to -1.0 to 1.0, which represent looking straight down and
	 * straight up, respectively. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Character Transform")
	float NormalizedCameraPitch = 0.0f;



	// Spring interpolation.

// Movement sway.
protected:

	/** The current spring value for the forward/backward movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Spring Values")
	float CurrentSpringMoveForwardBackward;

	/** The current spring value for the right/left movement sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Spring Values")
	float CurrentSpringMoveRightLeft;

	/** Spring state for the forward/backward movement sway's spring interpolation calculation. */
	FFloatSpringState SpringStateMoveForwardBackward;

	/** Spring state for the right/left movement sway's spring interpolation calculation. */
	FFloatSpringState SpringStateMoveRightLeft;

// Aim sway.
protected:

	/** The current spring value for the up/down aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Spring Values")
	float CurrentSpringAimUpDown;

	/** The current spring value for the right/left aim sway spring. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Spring Values")
	float CurrentSpringAimRightLeft;

	/** Spring state for the up/down aim sway's spring interpolation calculation. */
	FFloatSpringState SpringStateAimUpDown;

	/** The spring state variable used for the right/left aim sway's spring interpolation calculation. */
	FFloatSpringState SpringStateAimRightLeft;
};
