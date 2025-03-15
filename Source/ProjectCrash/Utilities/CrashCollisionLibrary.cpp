// Copyright Samuel Reitich. All rights reserved.

#include "Utilities/CrashCollisionLibrary.h"

#include "CrashMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

bool UCrashCollisionLibrary::RadialGroundDetection(const UObject* WorldContextObject, const FVector Position, const float Radius, const float Height, const float Midpoint, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, TArray<AActor*>& OutActors)
{
	OutActors.Empty();

	/* PhysX doesn't have a cylinder primitive, but we can simulate one by intersecting a capsule and a box. */
	const float HalfHeight = (Height / 2.0f);
	const float CapsuleHalfHeight = Radius + HalfHeight; // The capsule's height is immaterial (the box clamps the overlap's height), but we still need to make sure it's larger its radius.
	const FVector BoxExtent = FVector(Radius, Radius, HalfHeight);
	const float ScaledZ = (Position.Z + (Height * (-Midpoint + 0.5))); // Desired Z based on midpoint and given Z.
	const FVector Origin = FVector(Position.X, Position.Y, ScaledZ);
	TArray<AActor*> IgnoreActors;

	TArray<AActor*> CapsuleOverlapActors;
	bool CapsuleOverlapped = UKismetSystemLibrary::CapsuleOverlapActors(WorldContextObject, Origin, Radius, CapsuleHalfHeight, ObjectTypes, NULL, IgnoreActors, CapsuleOverlapActors);
	if (!CapsuleOverlapped)
	{
		return false;
	}

	TArray<AActor*> BoxOverlapActors;
	bool BoxOverlapped = UKismetSystemLibrary::BoxOverlapActors(WorldContextObject, Origin, BoxExtent, ObjectTypes, NULL, IgnoreActors, BoxOverlapActors);
	if (!BoxOverlapped)
	{
		return false;
	}

	// TODO: Debugging
	DrawDebugBox(WorldContextObject->GetWorld(), Origin, BoxExtent, FColor::Red, true);
	DrawDebugCapsule(WorldContextObject->GetWorld(), Origin, CapsuleHalfHeight, Radius, FQuat::MakeFromRotator(FRotator::ZeroRotator), FColor::Yellow, true);
	DrawDebugCylinder(WorldContextObject->GetWorld(), Origin - FVector(0, 0, HalfHeight), Origin + FVector(0, 0, HalfHeight), Radius, 32, FColor::Green, true);

	// Actors within our imaginary cylinder.
	TArray<AActor*> UnfilteredActors = UCrashMathLibrary::Intersect<AActor*>(CapsuleOverlapActors, BoxOverlapActors);

	for (AActor* Actor : UnfilteredActors)
	{
		// Check for ground.
	}

	return (OutActors.Num() > 0);
}
