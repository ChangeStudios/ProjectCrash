// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystemUtilitiesLibrary.generated.h"

/**
 * Collection of utilities for scripting gameplay abilities.
 */
UCLASS(Meta = (BlueprintThreadSafe, ScriptName = "AbilitySystemUtilitiesLibrary"), MinimalAPI)
class UAbilitySystemUtilitiesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Tests if two actors have line-of-sight to each other by performing a line trace between their root components.
	 * Returns false if the trace hits anything blocking the given channel.
	 *
	 * @param SourceActor		First actor.
	 * @param TargetActor		Second actor.
	 * @param TraceChannel		Channel to trace for.
	 * @param bIgnorePawns		If true, ignores any pawns blocking the traces between A and B.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision", DisplayName = "Has Line of Sight?", Meta = (WorldContext = "WorldContextObject", Keywords = "los ray cast trace"))
	static bool HasLineOfSight(const UObject* WorldContextObject, AActor* SourceActor, AActor* TargetActor, ETraceTypeQuery TraceChannel, bool bIgnorePawns = true);

	/**
	 * Applies knockback to the target actor in the direction of the source to the target.
	 *
	 * @param VelocityMagnitude		Scalar applied to knockback direction, in cm/s.
	 * @param Instigator			The actor responsible for applying this knockback. If it was applied by an ability,
	 *								this should be the ASC's owner.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability Statics|Knockback")
	static void ApplyKnockbackToTargetFromLocation(float VelocityMagnitude, FVector Source, AActor* Target, AActor* Instigator);

	/**
	 * Applies directional knockback to the target actor.
	 *
	 * @param Velocity				Direction and magnitude of knockback, in cm/s.
	 * @param Instigator			The actor responsible for applying this knockback. If it was applied by an ability,
	 *								this should be the ASC's owner.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability Statics|Knockback")
	static void ApplyKnockbackToTargetInDirection(FVector Velocity, AActor* Target, AActor* Instigator);
};
