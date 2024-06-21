// Copyright Samuel Reitich. All rights reserved.


#include "BlueprintLibraries/AbilitySystemUtilitiesLibrary.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

bool UAbilitySystemUtilitiesLibrary::Actor_HasLineOfSight(const UObject* WorldContextObject, AActor* StartingActor, AActor* TargetActor, ETraceTypeQuery TraceChannel, bool bIgnorePawns)
{
	// Ensure the given actors are valid.
	if (!StartingActor || !TargetActor)
	{
		return false;
	}

	// Use the actors' locations to test for LoS.
	const FVector Start = StartingActor->GetActorLocation();
	const FVector EndCenter = TargetActor->GetActorLocation();

	// Ignore the two actors we're tracing between.
	const TArray<AActor*> ActorsToIgnore = {StartingActor, TargetActor};

	// Perform a trace between the roots of the actors.
	const bool bCenterTraceResult = HasLineOfSight_Internal(WorldContextObject, Start, EndCenter, ActorsToIgnore, TraceChannel, bIgnorePawns);

	/* If the target is a pawn or has a capsule component, perform two additional traces: one to the top of the actor
	 * and one to the bottom of the actor. Otherwise, only perform the center trace. */
	const bool bTraceToTopAndBottom = Cast<const APawn>(TargetActor) || (Cast<UCapsuleComponent>(TargetActor->GetRootComponent()) != NULL);

	FVector EndTop;
	FVector EndBottom;

	if (bTraceToTopAndBottom)
	{
		// Form a collision cylinder around the target actor to find its height.
		float OtherRadius, OtherHalfHeight;
		TargetActor->GetSimpleCollisionCylinder(OtherRadius, OtherHalfHeight);

		// Calculate the top and bottom of the target actor.
		EndTop = TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, OtherHalfHeight);
		EndBottom = TargetActor->GetActorLocation() - FVector(0.0f, 0.0f, OtherHalfHeight);
	}

	// Trace to the top and bottom of the target actor.
	const bool bTopTraceResult = bTraceToTopAndBottom ? HasLineOfSight_Internal(WorldContextObject, Start, EndTop, ActorsToIgnore, TraceChannel, bIgnorePawns) : true;
	const bool bBottomTraceResult = bTraceToTopAndBottom ? HasLineOfSight_Internal(WorldContextObject, Start, EndBottom, ActorsToIgnore, TraceChannel, bIgnorePawns) : true;

	/* An actor has line-of-sight if it has LoS to either the top half of an actor (center and top) or the bottom half
	 * of an actor (center and bottom). */
	return (bCenterTraceResult) && (bTopTraceResult || bBottomTraceResult);
}

bool UAbilitySystemUtilitiesLibrary::Vector_HasLineOfSight(const UObject* WorldContextObject, const FVector A, const FVector B, ETraceTypeQuery TraceChannel, bool bIgnorePawns)
{
	// Perform a trace between the two given vectors.
	return HasLineOfSight_Internal(WorldContextObject, A, B, TArray<AActor*>(), TraceChannel, bIgnorePawns);
}

bool UAbilitySystemUtilitiesLibrary::HasLineOfSight_Internal(const UObject* WorldContextObject, FVector A, FVector B, const TArray<AActor*>& ActorsToIgnore, ETraceTypeQuery TraceChannel, bool bIgnorePawns)
{
	TArray<FHitResult> OutHits;

	// Perform a trace between the given locations.
	UKismetSystemLibrary::LineTraceMulti
	(
		WorldContextObject,
		A,
		B,
		TraceChannel,
		true,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutHits,
		false
	);

	// Iterate through each hit
	for (FHitResult HitResult : OutHits)
	{
		if (HitResult.bBlockingHit)
		{
			// If we're ignoring pawns and the trace hit a pawn, ignore it.
			if (bIgnorePawns && HitResult.GetActor() && HitResult.GetActor()->IsA(APawn::StaticClass()))
			{
				continue;
			}
			// If we're not doing any filtering, any blocking hit will block LoS.
			else
			{
				return false;
			}
		}
	}

	return true;
}
