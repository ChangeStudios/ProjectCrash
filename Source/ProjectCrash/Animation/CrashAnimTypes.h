// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CrashAnimTypes.generated.h"

class UAnimMontage;

/**
 * Structure containing a pair of animation assets: one for first-person and one for third-person.
 */
USTRUCT(BlueprintType)
struct FAnimPerspectivePair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Animation")
	TObjectPtr<UAnimMontage> Anim_FPP;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Animation")
	TObjectPtr<UAnimMontage> Anim_TPP;
};