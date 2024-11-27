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
		DrawDebugLine(World, ViewLocation, ViewEnd, FColor::Green, false, 2.0f);
	}
#endif // ENABLE_DRAW_DEBUG

	return HitResults;
}
