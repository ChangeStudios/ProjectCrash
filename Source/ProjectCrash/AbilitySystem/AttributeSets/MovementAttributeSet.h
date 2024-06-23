// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "MovementAttributeSet.generated.h"

/**
 * Attribute set that determines characters' movement properties. Requires that the ASC's avatar has a movement
 * component. 
 */
UCLASS()
class PROJECTCRASH_API UMovementAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()
};
