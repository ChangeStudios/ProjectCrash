// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_ContinuousTrace.generated.h"

/**
 * Base class for target actors that continuously perform traces while active. This should be subclassed with custom
 * trace shapes. By default, this class uses its owning player's camera view to direct the trace.
 *
 * This is a rewrite of the AGameplayAbilityTargetActor_Trace to add support for multiple hits and to format the class
 * with this project's standards.
 */
UCLASS(Abstract, NotPlaceable)
class PROJECTCRASH_API AGameplayAbilityTargetActor_ContinuousTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

	// Construction.
	
public:

	AGameplayAbilityTargetActor_ContinuousTrace(const FObjectInitializer& ObjectInitializer);



	// Targeting

public:

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	// Stops targeting without firing "Canceled" delegates.
	UFUNCTION(BlueprintCallable)
	virtual void StopTargeting();

	virtual void CancelTargeting() override;

	virtual void Tick(float DeltaSeconds) override;



	// Parameters.

public:

	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true"))
	float MaxRange;

	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true"))
	TEnumAsByte<ECollisionChannel> TraceChannel;



	// Internals

protected:

	/** Performs the actual trace. Overridden in subclasses for custom trace shapes. */
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor) PURE_VIRTUAL(AGameplayAbilityTargetActor_ContinuousTrace, return TArray<FHitResult>(); );

	/** Updates the given hit result's location, which is used to determine the location of VFX. By default, if the hit
	 * actor is a CrashCharacter, sets the location to the location of the closest bone to the original trace vector. */
	virtual void OverrideHitLocation(FHitResult& HitResult);

private:

	// Targets that have been hit. used to avoid repeating targets within a single activation.
	UPROPERTY()
	TArray<const AActor*> HitTargets;
};
