// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
	 * @param OutActors					Returned array of actors, unsorted.
	 * @return							True if there was at least one successful overlap. False otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision", Meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore", DisplayName = ""), Meta = (Keywords = "overlap, collision, radius, sphere, circle, cylinder"))
	static bool RadialGroundDetection(const UObject* WorldContextObject, const FVector Position, const float Radius, const float Height, const float Midpoint, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, TArray<AActor*>& OutActors);
};
