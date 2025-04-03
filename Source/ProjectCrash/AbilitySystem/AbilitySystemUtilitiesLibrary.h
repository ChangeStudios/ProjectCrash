// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystemUtilitiesLibrary.generated.h"

struct FCrashTargetDataFilter;

/**
 * Collection of utilities for scripting gameplay abilities.
 */
UCLASS(Meta = (BlueprintThreadSafe, ScriptName = "AbilitySystemUtilitiesLibrary"), MinimalAPI)
class UAbilitySystemUtilitiesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Gameplay.

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



	// Effects.

public:

	/**
	 * Retrieves the first hit result from the target data and overrides its Normal value.
	 *
	 * This is used when we want to set a custom direction for target data (i.e. when we want to apply knockback in a
	 * direction other than the target data's), but still want to use the target data's hit result (e.g. for VFX).
	 */
	UFUNCTION(BlueprintPure, Category = "Ability|TargetData")
	static FHitResult GetTargetDataHitResultWithCustomDirection(const FGameplayAbilityTargetDataHandle& TargetData, FVector NewDirection);



	// Target data.

public:

	/**
	 * Create a handle for filtering target data.
	 *
	 * @param Filter		The filter to create.
	 * @param FilterActor	The source actor of the filter. When a filter refers to "self," it refers to this actor.
	 */
	UFUNCTION(BlueprintPure, Category = "Filter")
	static FGameplayTargetDataFilterHandle MakeCrashFilterHandle(FCrashTargetDataFilter Filter, AActor* FilterActor);
};
