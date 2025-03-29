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

	// Collision detection.

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



	// Collision tracing.

public:

	/** Sweeps a sphere, trying to find ANY path along the trace that doesn't get blocked. Returns true if there exists
	 * any horizontal path from the starting sphere to the ending sphere that is not blocked. */
	bool SphereSweepForAnyPath(const UObject* WorldContextObject, float SphereRadius, float Subdivisions, const FVector Start, const FVector End, ECollisionChannel TraceChannel, FCollisionQueryParams& CollisionQueryParams);

	UFUNCTION(BlueprintCallable, Category = "Collision", Meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "Cone Trace By Channel", AdvancedDisplay = "TraceColor, TraceHitColor, DrawTime", Keywords = "sweep"))
	static bool ConeTraceSingle(const UObject* WorldContextObject, const FVector Start, const FRotator Direction, float ConeHeight, float ConeHalfAngle, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f) { return false; }

	UFUNCTION(BlueprintCallable, Category = "Collision", Meta = (bIgnoreSelf = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", DisplayName = "Multi Cone Trace By Channel", AdvancedDisplay = "TraceColor, TraceHitColor, DrawTime", Keywords = "sweep"))
	static bool ConeTraceMulti(const UObject* WorldContextObject, const FVector Start, const FRotator Direction, float ConeHeight, float ConeHalfAngle, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType, TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);



	// Utils.

public:

	/**
	 * Return the actor whose hit normal is closest to given direction vector.
	 *
	 * @param Aim				Direction vector to compare hits to. The hit with the normal closest to this will be
	 *							returned.
	 * @param Origin			The origin of the aim direction (usually the aiming player's view location). Used to
	 *							calculate each hit's normal. The result's given normal/impact normal may not be
	 *							accurate, depending on the shape that was used to generate it.
	 * @param bCheckLineOfSight If true, hit actors must have LOS. Helps ensure that something closer was not being
	 *							aimed at.
	 * @param IgnoreActor		Actor to ignore when checking LOS (the source actor, e.g. instigating player character).
	 * @param MinimumDot		The dot product between hits and the aim vector must be greater than this. Helps ensure
	 *							the target was actually being aimed at.
	 * @param Hits				Array of hits to check.
	 * @param FilterHandle		Optional target data filter.
	 */
	UFUNCTION(BlueprintPure, Category = "Collision", Meta = (bCheckLineOfSight = "true", MinimumDot = "0.5", AdvancedDisplay = "MinimumDot", Keywords = "find normal align"))
	AActor* GetHitClosestToAim(FVector Aim, FVector Origin, bool bCheckLineOfSight, AActor* IgnoreActor, float MinimumDot, TArray<FHitResult>& Hits, FGameplayTargetDataFilterHandle FilterHandle);

	/**
	 * Creates an instance of FCollisionQueryParams using a given context object. The params' "Self" will be the context
	 * object if it's an actor; otherwise, it will be the first actor in the object's owner chain.
	 *
	 * NOTE: This is a copy of the ConfigureCollisionParams function in KismetTraceUtils.h, which Unreal does not allow
	 * to be used by external modules, for some reason.
	 */
	static FCollisionQueryParams ConfigureCollisionParams(FName TraceTag, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, bool bIgnoreSelf, const UObject* WorldContextObject);
};
