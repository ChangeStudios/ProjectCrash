// Copyright Samuel Reitich. All rights reserved.

#include "Utilities/CrashCollisionLibrary.h"

#include "CrashMathLibrary.h"
#include "KismetTraceUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

bool UCrashCollisionLibrary::RadialGroundDetection(const UObject* WorldContextObject, const FVector Position, const float Radius, const float Height, const float Midpoint, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore, const bool bLimitClimbing, const float MaxClimb, EDrawDebugTrace::Type DrawDebugType, float DrawTime, TArray<AActor*>& OutActors)
{
	OutActors.Empty();

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return false;
	}

	/* Chaos doesn't have a cylinder primitive, but we can simulate one by intersecting a capsule and a box. */
	const float HalfHeight = (Height / 2.0f);
	const float CapsuleHalfHeight = Radius + HalfHeight; // The capsule's height is immaterial (the box clamps the overlap's height), but we still need to make sure it's larger than its radius.
	const FVector BoxExtent = FVector(Radius, Radius, HalfHeight);
	const float ScaledZ = (Position.Z + (Height * (-Midpoint + 0.5))); // Desired Z based on midpoint and given Z.
	const FVector Origin = FVector(Position.X, Position.Y, ScaledZ);

	TArray<AActor*> CapsuleOverlapActors;
	bool CapsuleOverlapped = UKismetSystemLibrary::CapsuleOverlapActors(WorldContextObject, Origin, Radius, CapsuleHalfHeight, ObjectTypes, NULL, ActorsToIgnore, CapsuleOverlapActors);
	if (!CapsuleOverlapped)
	{
		return false;
	}

	TArray<AActor*> BoxOverlapActors;
	bool BoxOverlapped = UKismetSystemLibrary::BoxOverlapActors(WorldContextObject, Origin, BoxExtent, ObjectTypes, NULL, ActorsToIgnore, BoxOverlapActors);
	if (!BoxOverlapped)
	{
		return false;
	}

#if ENABLE_DRAW_DEBUG
	bool bDebugPersistent = (DrawDebugType == EDrawDebugTrace::Persistent);
	float DebugLifetime = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawTime : -1.0f;

	if (DrawDebugType != EDrawDebugTrace::None)
	{
		DrawDebugCylinder(WorldContextObject->GetWorld(), Origin - FVector(0, 0, HalfHeight), Origin + FVector(0, 0, HalfHeight), Radius, 32, FColor::Green, bDebugPersistent, DebugLifetime);
	}
#endif // ENABLE_DRAW_DEBUG

	// Actors within our imaginary cylinder.
	TArray<AActor*> UnfilteredActors = UCrashMathLibrary::Intersect<AActor*>(CapsuleOverlapActors, BoxOverlapActors);

	// ---------------------------------
	// Check if we can hit the actor. We can hit any actors if we can reach the bottom of the actor from the center of
	// the cylinder without being fully blocked by anything. We can incrementally climb over obstacles (e.g. stairs,
	// ramps, uneven terrain, etc.) to reach the target, as long as we don't exit the original cylinder or exceed our
	// maximum climbing distance.
	//

	for (AActor* Actor : UnfilteredActors)
	{
		/* Find the bottom of the actor. If the actor is a character, use the bottom of their capsule. Otherwise, just
		 * use the actor's location. */
		FVector TargetLocation = Actor->GetActorLocation();

		if (ACharacter* TargetAsCharacter = Cast<ACharacter>(Actor))
		{
			TargetLocation.Z = TargetLocation.Z - TargetAsCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}

		// An actor's hitbox may be in range, but the bottom of it is too far down.
		if (abs(Origin.Z - TargetLocation.Z) > HalfHeight)
		{
#if ENABLE_DRAW_DEBUG
			if (DrawDebugType != EDrawDebugTrace::None)
			{
				DrawDebugDirectionalArrow(World, Actor->GetActorLocation(), TargetLocation, 50.0f, FColor::Red, bDebugPersistent, DebugLifetime, -1, 1.0f);
			}
#endif // ENABLE_DRAW_DEBUG
			continue;
		}

		FVector CurrentPosition(Origin);
		const float Top = Origin.Z + HalfHeight;

		ECollisionChannel TraceChannel = ECC_Camera;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(RadialGroundDetection), true);
		Params.AddIgnoredActors(ActorsToIgnore);
		const float Step = 1.0f;
		float CurrentClimb = 0.0f;
		const float EffectiveMaxClimb = bLimitClimbing ? MaxClimb : HalfHeight;

		while ((CurrentPosition.Z < Top) && (CurrentClimb < EffectiveMaxClimb))
		{
			// Trace horizontally towards the target.
			FHitResult HorizontalHitResult;
			FVector HorizontalStart = CurrentPosition;
			FVector HorizontalEnd = FVector(TargetLocation.X, TargetLocation.Y, CurrentPosition.Z);
			World->LineTraceSingleByChannel(HorizontalHitResult, HorizontalStart, HorizontalEnd, TraceChannel, Params);
			CurrentPosition = (HorizontalHitResult.bBlockingHit ? HorizontalHitResult.ImpactPoint : HorizontalEnd);

#if ENABLE_DRAW_DEBUG
			if (DrawDebugType != EDrawDebugTrace::None)
			{
				DrawDebugLine(World, HorizontalStart, CurrentPosition, FColor::Yellow, bDebugPersistent, DebugLifetime, 0, 1.0f);
			}
#endif // ENABLE_DRAW_DEBUG

			// If we have reached our target horizontally, trace vertically to try to reach the bottom of the target.
			if (!HorizontalHitResult.bBlockingHit)
			{
				FHitResult VerticalHitResult;
				FVector VerticalStart = CurrentPosition;
				FVector VerticalEnd = FVector(CurrentPosition.X, CurrentPosition.Y, TargetLocation.Z);
				World->LineTraceSingleByChannel(VerticalHitResult, VerticalStart, VerticalEnd, TraceChannel, Params);

#if ENABLE_DRAW_DEBUG
				if (DrawDebugType != EDrawDebugTrace::None)
				{
					DrawDebugLine(World, VerticalStart, (VerticalHitResult.bBlockingHit ? VerticalHitResult.ImpactPoint : VerticalEnd), FColor::Yellow, bDebugPersistent, DebugLifetime, 0, 1.0f);
				}
#endif // ENABLE_DRAW_DEBUG

				if (!VerticalHitResult.bBlockingHit)
				{
					OutActors.Add(Actor);
					break;
				}
			}
			else
			{
				// Pull back a little bit to avoid clipping into whatever's just blocked us.
				CurrentPosition = CurrentPosition - (HorizontalEnd - HorizontalStart).GetSafeNormal2D();

				// If we made some horizontal progress, reset our current climb.
				if (FVector::Dist(HorizontalStart, CurrentPosition) > 0.2f)
				{
					CurrentClimb = 0.0f;
				}
				// Increment our current climb if we aren't making any horizontal progress.
				else
				{
					CurrentClimb += Step;
				}

				/* If we can't get to our target horizontally, incrementally climb up to see if we can get over whatever is
				 * blocking us (stairs, ramps, uneven terrain, etc.). */
				CurrentPosition.Z += Step;
			}
		}

#if ENABLE_DRAW_DEBUG
		if (OutActors.Contains(Actor))
		{
			// Success
			if (DrawDebugType != EDrawDebugTrace::None)
			{
				DrawDebugPoint(World, TargetLocation, 25.0f, FColor::Green, bDebugPersistent, DebugLifetime, 0);
			}
		}
		else
		{
			// Fail
			if (DrawDebugType != EDrawDebugTrace::None)
			{
				DrawDebugPoint(World, CurrentPosition, 25.0f, FColor::Red, bDebugPersistent, DebugLifetime, 0);
			}
		}
#endif // ENABLE_DRAW_DEBUG
	}

	return (OutActors.Num() > 0);
}

bool UCrashCollisionLibrary::SphereSweepForAnyPath(const UObject* WorldContextObject, float SphereRadius, float Subdivisions, const FVector Start, const FVector End, ECollisionChannel TraceChannel, FCollisionQueryParams& CollisionQueryParams)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World))
	{
		return false;
	}

	int32 NumPoints = FMath::Sqrt(float(Subdivisions));
	for (int32 i = 0; i < NumPoints; ++i)
	{
		for (int32 j = 0; j < NumPoints; ++j)
		{
			// Generate a set of points in the start and end spheres.
			float StartPhi = i / (NumPoints - 1.0) * PI;
			float StartTheta = j / (NumPoints - 1.0) * 2.0 * PI;
			FVector StartSpherePoint = Start + FVector(
				SphereRadius * sin(StartPhi) * cos(StartTheta),
				SphereRadius * sin(StartPhi) * sin(StartTheta),
				SphereRadius * cos(StartPhi)
			);
	
			float EndPhi = i / (NumPoints - 1.0) * PI;
			float EndTheta = j / (NumPoints - 1.0) * 2.0 * PI;
			FVector EndSpherePoint = End + FVector(
				SphereRadius * sin(EndPhi) * cos(EndTheta),
				SphereRadius * sin(EndPhi) * sin(EndTheta),
				SphereRadius * cos(EndPhi)
			);
	
			// Perform a line trace between our points.
			FHitResult LineHitResult;
			bool bHit = World->LineTraceSingleByChannel(
				LineHitResult,
				StartSpherePoint,
				EndSpherePoint,
				TraceChannel,
				CollisionQueryParams
			);

			if (!bHit)
			{
				return true;
			}
		}
	}

	return false;
}
