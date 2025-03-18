// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CrashCollisionLibrary.generated.h"

/**
 * Utilities for collision.
 */
UCLASS(MinimalAPI)
class UCrashCollisionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Performs a cylindrical overlap for actors that are on the ground.
	 *
	 * @param WorldContextObject		World context.
	 * @param Position					Center of the cylinder.
	 * @param Radius					Radius of the cylinder.
	 * @param Height					Full height of the cylinder.
	 * @param Midpoint					Where along the cylinder's vertical axis the position is relative to. 0 means
	 *									Position is the bottom of the cylinder; 1 means it's the top.
	 * @param ObjectTypes				Object types to check for.
	 * @param ActorsToIgnore			Actors to ignore.
	 * @param bLimitClimbing			If true, the trace will only be able to climb a certain distance upwards,
	 *									without any horizontal progress, before giving up. This allows the trace to
	 *									climb over stairs, ramps, uneven terrain, etc., but not walls and other vertical
	 *									obstacles.
	 * @param MaxClimb					If bLimitClimbing is enabled, this is the maximum vertical distance traces are
	 *									allowed to travel, without any horizontal progress, before giving up. If
	 *									bLimitClimbing is disabled, the effective MaxClimb will be (Height / 2.0).
	 * @param DrawDebugType				Enable/disable debug drawing.
	 * @param DrawTime					Duration of debug draw if draw type is ForDuration.
	 * @param OutActors					Returned array of actors, unsorted.
	 * @return							True if there was at least one successful overlap. False otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision", Meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"), Meta = (Midpoint = "0.5", Keywords = "overlap, collision, radius, sphere, circle, cylinder, trace", AdvancedDisplay = "bLimitClimbing, MaxClimb, DrawDebugType, DrawTime"))
	static bool RadialGroundDetection(const UObject* WorldContextObject, const FVector Position, const float Radius, const float Height, const float Midpoint, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore, const bool bLimitClimbing, const float MaxClimb, EDrawDebugTrace::Type DrawDebugType, float DrawTime, TArray<AActor*>& OutActors);

	/** Sweeps a sphere, trying to find ANY path along the trace that doesn't get blocked. Returns true if there exists
	 * any horizontal path from the starting sphere to the ending sphere that is not blocked. */
	bool SphereSweepForAnyPath(const UObject* WorldContextObject, float SphereRadius, float Subdivisions, const FVector Start, const FVector End, ECollisionChannel TraceChannel, FCollisionQueryParams& CollisionQueryParams);
};
