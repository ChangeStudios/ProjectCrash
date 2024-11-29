// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_ContinuousTrace_Cone.h"

#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

TArray<FHitResult> AGameplayAbilityTargetActor_ContinuousTrace_Cone::PerformTrace(AActor* InSourceActor)
{
	UWorld* World = InSourceActor->GetWorld();

	check(World);

	if (!OwningAbility) // Only valid on server and owning client
	{
		return TArray<FHitResult>();
	}

	TArray<FHitResult> HitResults;
	bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_ContinuousTrace_Cone), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(InSourceActor);

	APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector ViewEnd = ViewLocation + (ViewRotation.Vector() * MaxRange);

	// FCollisionShape SphereSweep = FCollisionShape::MakeCapsule()

	World->LineTraceMultiByChannel(HitResults, ViewLocation, ViewEnd, TraceChannel, Params);

	// World->SweepMultiByChannel(HitResults, ViewLocation, ViewEnd, ViewRotation, TraceChannel, )

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		// Debug lines for every hit in this frame's sphere sweep.
		// for (const FHitResult& HitResult : HitResults)
		// {
		// 	FVector HitDirection = (HitResult.ImpactPoint - ViewLocation);
		// 	HitDirection.Normalize();
		// 	DrawDebugLine(World, ViewLocation, ViewLocation + (HitDirection * MaxRange), FColor::Green, true);
		// }

		/* This debug draw shows the sphere sweep we've actually performed, not the simulated cone that is used to
		 * determine whether a hit is valid. This is primarily for backend debugging to make sure the theoretical cone
		 * we want to simulate aligns with the trace we performed. */
		DrawDebugSphereTraceMulti(World, ViewLocation, ViewEnd, ConeBaseRadius, EDrawDebugTrace::Persistent, false, HitResults, FColor::Red, FColor::Red, -1.0f);

		// Debug cone showing what the cone we're trying to simulate would look like.
		DrawDebugCone(World, ViewLocation, ViewRotation.Vector(), ConeSlantHeight, ConeHalfAngleRad, ConeHalfAngleRad, 24, FColor::Green, true);
	}
#endif // ENABLE_DRAW_DEBUG

	return HitResults;
}
