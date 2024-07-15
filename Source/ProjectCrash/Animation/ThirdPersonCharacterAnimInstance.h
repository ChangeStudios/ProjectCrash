// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/CharacterAnimInstanceBase.h"
#include "ThirdPersonCharacterAnimInstance.generated.h"

/** Represents one of four cardinal directions. Used for selecting 4D locomotion animations. */
UENUM(BlueprintType)
enum FCardinalDirection : uint8
{
	North,
	South,
	East,
	West
};



/**
 * Base animation instance for third-person animation blueprints. Collects additional data used for animating
 * characters in third-person.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UThirdPersonCharacterAnimInstance : public UCharacterAnimInstanceBase
{
	GENERATED_BODY()
};
