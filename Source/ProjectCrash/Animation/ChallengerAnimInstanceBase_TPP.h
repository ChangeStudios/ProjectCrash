// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChallengerAnimInstanceBase.h"
#include "ChallengerAnimInstanceBase_TPP.generated.h"

class AChallengerBase;

/**
 * Base animation instance for challenger characters' third-person animation blueprints.
 */
UCLASS()
class PROJECTCRASH_API UChallengerAnimInstanceBase_TPP : public UChallengerAnimInstanceBase
{
	GENERATED_BODY()

	// Animation updating.

public:

	/** Thread-safe animation update function. This is used instead of NativeUpdateAnimation to utilize multi-threading,
	 * avoiding a major bottleneck caused by traditional animation updating. This function can only call other thread-
	 * safe functions. */
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:

	/** Update the character's base pose using the player's current aiming pitch. */
	void UpdateAimOffset();



	// Character transform values updated each animation cycle to perform animation logic.

// Aim & rotation.
protected:

	/** The character's aim rotation pitch normalized to -1.0 to 1.0, which represent looking straight down and
	 * straight up, respectively. Only used for remote clients because the local client can access their own camera's
	 * aim rotation directly, which is faster for them. */
	UPROPERTY(BlueprintReadOnly, Category = "Characters|Challenger|Animation|Character Transform")
	float NormalizedAimPitch = 0.0f;
};
