// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_ContinuousTrace_Cone.h"

#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#define DEBUG_DRAW_TIME 3.0f
#define PROXIMITY_FORGIVENESS_TO_RADIUS 0.25f

TArray<FHitResult> AGameplayAbilityTargetActor_ContinuousTrace_Cone::PerformTrace(AActor* InSourceActor)
{
#if ENABLE_DRAW_DEBUG
	TArray<FVector> MissedHits;
	TArray<FVector> SuccessfulUnprocessedHits; // Successful hits that were thrown out for more favorable ones.
#endif // ENABLE_DRAW_DEBUG

	UWorld* World = InSourceActor->GetWorld();

	check(World);

	if (!OwningAbility) // Only valid on server and owning client
	{
		return TArray<FHitResult>();
	}

	TArray<FHitResult> HitResults;
	TArray<FHitResult> SuccessfulHits;
	bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_ContinuousTrace_Cone), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(InSourceActor);

	APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector ViewEnd = ViewLocation + (ViewRotation.Vector() * MaxRange);

	const double ConeHalfAngleRad = FMath::DegreesToRadians(ConeHalfAngle);
	const double ConeBaseRadius = MaxRange * tan(ConeHalfAngleRad); // r = h * tan(theta / 2)
	const double ConeSlantHeight = FMath::Sqrt((ConeBaseRadius * ConeBaseRadius) + (MaxRange * MaxRange)); // s = sqrt(r^2 + h^2)

	// If the hit is within this distance (really close), we won't check if it's within the cone, as long as it's in front of us (normal < 180 deg).
	const float ProximityForgivenessDistance = ConeBaseRadius * PROXIMITY_FORGIVENESS_TO_RADIUS;

	/* Unreal doesn't support cone collision shapes, and FCollisionShape isn't well-suited to being extended. To perform
	 * a cone-shaped trace, we perform a sphere sweep with the radius of the cone's base, and then check each hit to see
	 * if it would be inside a theoretical cone, depending on its distance and normal to our trace origin. We could also
	 * do a capsule overlap, but a sweep gets us better hit results for things like damage VFX. */
	FCollisionShape SphereSweep = FCollisionShape::MakeSphere(ConeBaseRadius);
	World->SweepMultiByChannel(HitResults, ViewLocation, ViewEnd, ViewRotation.Quaternion(), TraceChannel, SphereSweep, Params);

	for (FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();

		if (!IsValid(HitActor))
		{
			continue;
		}

		// HitResult.Distance gives us the distance of the sweep trace, but we want the distance to the impact point.
		const double Distance = (HitResult.ImpactPoint - ViewLocation).Length();
		/* Normally, the hit needs to be within the cone if we want to count it. But if we're REALLY close to our
		 * target, we'll count the hit even if it's outside the cone, as long as it's in front of us. */
		const double MaxAngle = (Distance < ProximityForgivenessDistance) ? (PI / 2.0) : ConeHalfAngleRad;

		// Check if the hit is within the angle of the simulated cone.
		const FVector HitDirection = (HitResult.ImpactPoint - ViewLocation);
		const double Dot = FVector::DotProduct(ViewRotation.Vector(), HitDirection.GetSafeNormal());
		const double DeltaAngle = FMath::Acos(Dot);		// theta = arccos ( (A • B) / (|A|*|B|) )		(|A|*|B| = 1, A and B are unit vectors)
		if (DeltaAngle > MaxAngle)
		{
#if ENABLE_DRAW_DEBUG
			MissedHits.Add(HitResult.ImpactPoint);
#endif

			continue;
		}

		/* Since we're using a sphere sweep, we may get hits in the hemisphere of the capsule formed by the sweep, which
		 * would be beyond the simulated cone's flat base. */
		const double LengthAtAngle = MaxRange / cos(DeltaAngle);	// hypotenuse = adjacent / cos(theta)
		if (Distance > LengthAtAngle)
		{
#if ENABLE_DRAW_DEBUG
			MissedHits.Add(HitResult.ImpactPoint);
#endif

			continue;
		}

		/* If we've already hit this actor, check the new hit against the existing one and use whichever hit is closer
		 * to our view direction, just so the VFX appear closer to where we're actually aiming. */
		const int32 ExistingHitIndex = SuccessfulHits.IndexOfByPredicate(
			[HitActor](const FHitResult& Hit){ return Hit.GetActor() == HitActor; });
		if (ExistingHitIndex != INDEX_NONE)
		{
			const FHitResult& ExistingHitOnActor = SuccessfulHits[ExistingHitIndex];

			const FVector ExistingDirection = (ExistingHitOnActor.ImpactPoint - ViewLocation);
			const double ExistingDot = FVector::DotProduct(ViewRotation.Vector(), ExistingDirection.GetSafeNormal());

			// If the new hit is closer to the player's aim, remove the old hit and use the new one.
			if (Dot > ExistingDot)
			{
#if ENABLE_DRAW_DEBUG
				SuccessfulUnprocessedHits.Add(ExistingHitOnActor.ImpactPoint);
#endif // ENABLE_DRAW_DEBUG

				SuccessfulHits.RemoveAt(ExistingHitIndex);
			}
			// If the new hit is further from the player's aim, throw it out.
			else
			{
#if ENABLE_DRAW_DEBUG
				SuccessfulUnprocessedHits.Add(HitResult.ImpactPoint);
#endif // ENABLE_DRAW_DEBUG

				continue;
			}
		}

		// We want the actual normal of the trace we performed; not whatever our sweep told us.
		HitResult.Normal = ViewRotation.Vector();
		HitResult.ImpactNormal = HitDirection.GetSafeNormal();

		SuccessfulHits.Add(HitResult);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		/* This debug draw shows the sphere sweep we've actually performed, not the simulated cone that is used to
		 * determine whether a hit is successful. This is primarily for backend debugging to make sure the theoretical
		 * cone we want to simulate aligns with the trace we performed. */
		// DrawDebugSphereTraceMulti(World, ViewLocation, ViewEnd, ConeBaseRadius, EDrawDebugTrace::ForDuration, false, HitResults, FColor::Red, FColor::Red, DEBUG_DRAW_TIME);

		// Debug cone showing what the cone we're trying to simulate would look like.
		DrawDebugCone(World, ViewLocation, ViewRotation.Vector(), ConeSlantHeight, ConeHalfAngleRad, ConeHalfAngleRad, 24, FColor(0, 128, 0), false, DEBUG_DRAW_TIME, 0, 0.0f);

		// Debug sphere showing the space where targets are so close that we don't care about their direction.
		DrawDebugSphere(World, ViewLocation, ProximityForgivenessDistance, 24, FColor(0, 128, 0), false, DEBUG_DRAW_TIME);

		// Debug lines for every unsuccessful hit we processed from the sweep.
		for (const FVector& MissedHit : MissedHits)
		{
			DrawDebugLine(World, ViewLocation, MissedHit, FColor::Red, false, DEBUG_DRAW_TIME);
		}

		// Debug lines for successful hits that were thrown out because we found a better one.
		for (const FVector& SuccessfulUnprocessedHit : SuccessfulUnprocessedHits)
		{
			DrawDebugLine(World, ViewLocation, SuccessfulUnprocessedHit, FColor::Green, false, DEBUG_DRAW_TIME);
		}

		// Debug lines for every successful hit we actually processed from the sweep.
		for (const FHitResult& SuccessfulHit : SuccessfulHits)
		{
			DrawDebugLine(World, ViewLocation, SuccessfulHit.ImpactPoint, FColor::Cyan, false, DEBUG_DRAW_TIME);
		}
	}
#endif // ENABLE_DRAW_DEBUG

	return SuccessfulHits;
}
