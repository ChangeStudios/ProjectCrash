// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ChallengerAnimInstanceBase.h"
#include "ChallengerAnimInstanceBase_FPP.generated.h"

class AChallengerBase;

/**
 * Base animation instance for challenger characters' first-person animation blueprints.
 */
UCLASS()
class PROJECTCRASH_API UChallengerAnimInstanceBase_FPP : public UChallengerAnimInstanceBase
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Default constructor. */
	virtual void NativeInitializeAnimation() override;

	/** Default initializer. */
	virtual void NativeBeginPlay() override;



	// Utils.

protected:

	/** This animation instance's owning pawn, cached for convenience. */
	UPROPERTY(BlueprintReadOnly, Category = "Heroes|Animation")
	TObjectPtr<AChallengerBase> OwningChallenger;



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

	/** The signed vector length of the character's current velocity. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Character Transform")
	float SignedSpeed;

// Aim & rotation.
protected:

	/** The character's camera aim rotation pitch normalized to -1.0 to 1.0, which represent looking straight down and
	 * straight up, respectively. */
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Character Transform")
	float NormalizedCameraPitch = 0.0f;
};
