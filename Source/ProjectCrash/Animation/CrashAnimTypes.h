// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CrashAnimTypes.generated.h"

class UAnimSequenceBase;

/**
 * Structure containing a pair of animation assets: one for first-person and one for third-person.
 */
USTRUCT(BlueprintType)
struct FAnimPerspectivePair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Animation")
	TObjectPtr<UAnimSequenceBase> Anim_FPP;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Animation")
	TObjectPtr<UAnimSequenceBase> Anim_TPP;
};