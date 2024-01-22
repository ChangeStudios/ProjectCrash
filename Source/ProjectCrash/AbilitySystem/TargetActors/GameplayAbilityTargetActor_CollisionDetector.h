// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_CollisionDetector.generated.h"

/**
 * TODO: Implement this to detect collision over a duration. Currently, we're performing traces each tick, which isn't performant. Implement a custom WaitTargetData task to accept multiple async target data returns.
 */
UCLASS()
class PROJECTCRASH_API AGameplayAbilityTargetActor_CollisionDetector : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

};