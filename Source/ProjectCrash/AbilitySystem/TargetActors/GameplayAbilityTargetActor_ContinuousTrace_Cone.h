// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_ContinuousTrace.h"
#include "GameplayAbilityTargetActor_ContinuousTrace_Cone.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, DisplayName = "Continuous Trace: Cone", Meta = (ScriptName = "Target Actor: Continuous Trace, Cone"))
class PROJECTCRASH_API AGameplayAbilityTargetActor_ContinuousTrace_Cone : public AGameplayAbilityTargetActor_ContinuousTrace
{
	GENERATED_BODY()

	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor) override;

	// Parameters.

protected:

	/** The radius of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true"))
	float ConeBaseRadius;

	/** The half-height of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true", ClampMin = "-180", ClampMax = "180", Units = "Degrees"))
	float ConeAngle;
};
