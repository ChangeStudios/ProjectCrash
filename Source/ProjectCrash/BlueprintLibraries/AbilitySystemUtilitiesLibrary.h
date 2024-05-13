// Copyright Samuel Reitich 2024.

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
	 * Tests if two actors have line-of-sight to each other by performing a line trace between them. Uses the location
	 * of the starting actor's root component and traces to the root of the target actor.
	 *
	 * If the target actor is a pawn or has a collision cylinder, traces to the top and bottom of the target are also
	 * performed. Returns true if there is line-of-sight to the top half of the target and/or the bottom half of the
	 * target.
	 *
	 * @param StartingActor		Actor to trace from.
	 * @param TargetActor		Actor to trace to.
	 * @param TraceChannel		Channel to trace for.
	 * @param bIgnorePawns		If true, ignores any pawns blocking the traces between A and B.
	 */
	UFUNCTION(BlueprintPure, Category = "Collision", Meta = (WorldContext = "WorldContextObject", DisplayName = "Has Line of Sight (Actor)", Keywords = "los raycast trace"))
	static PROJECTCRASH_API bool Actor_HasLineOfSight(const UObject* WorldContextObject, AActor* StartingActor, AActor* TargetActor, ETraceTypeQuery TraceChannel, bool bIgnorePawns = true);

	/**
	 * Tests if two vectors have line-of-sight to each other by performing a line trace between them.
	 *
	 * @param A					Actor to trace from.
	 * @param B					Actor to trace to.
	 * @param TraceChannel		Channel to trace for.
	 * @param bIgnorePawns		If true, ignores any pawns blocking the traces between A and B.
	 */
	UFUNCTION(BlueprintPure, Category = "Collision", Meta = (WorldContext = "WorldContextObject", DisplayName = "Has Line of Sight (Vector)", Keywords = "los raycast trace"))
	static PROJECTCRASH_API bool Vector_HasLineOfSight(const UObject* WorldContextObject, const FVector A, const FVector B, ETraceTypeQuery TraceChannel, bool bIgnorePawns = true);

	/** Performs a line trace between the two given vectors using the given parameters. */
	static bool HasLineOfSight_Internal(const UObject* WorldContextObject, FVector A, FVector B, const TArray<AActor*>& ActorsToIgnore, ETraceTypeQuery TraceChannel, bool bIgnorePawns);
};
