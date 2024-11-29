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

	/** The half-angle of the cone used for collision detection (degrees). */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true", ClampMin = "-90", ClampMax = "90", Units = "Degrees"))
	float ConeHalfAngle;
};
