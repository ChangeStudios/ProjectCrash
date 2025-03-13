// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_ContinuousTrace.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/CrashCharacter.h"

// Number of traces to perform per second.
#define TRACE_RATE 30.0f

/* The minimum dot product to a bone required to override our trace hit location with that bone's position. Otherwise,
 * we'll fall back to the location of the closest bone to the trace's origin. */
#define MIN_DOT_FOR_LOC_OVERRIDE 0.5

AGameplayAbilityTargetActor_ContinuousTrace::AGameplayAbilityTargetActor_ContinuousTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = (1.0f / TRACE_RATE);

	MaxRange = 500.0f;
}

void AGameplayAbilityTargetActor_ContinuousTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (!Ability)
	{
		return;
	}

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	// Reset cached targets so they can be hit again.
	HitTargets.Empty();

	// Start performing traces by ticking.
	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AGameplayAbilityTargetActor_ContinuousTrace::StopTargeting()
{
	// Stop performing traces.
	PrimaryActorTick.SetTickFunctionEnable(false);

	// Clear callbacks.
	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();

	if (GenericDelegateBoundASC)
	{
		GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(this, &ThisClass::ConfirmTargeting);
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &ThisClass::CancelTargeting);
		GenericDelegateBoundASC = nullptr;
	}
}

void AGameplayAbilityTargetActor_ContinuousTrace::CancelTargeting()
{
	// Perform cancellation-specific targeting-ending logic.
	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	UAbilitySystemComponent* ASC = (ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr);
	if (ASC)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey() ).Remove(GenericCancelHandle);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor_ContinuousTrace::CancelTargeting called with null ASC. Actor %s"), *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());

	// Clean up targeting actor.
	StopTargeting();
}

void AGameplayAbilityTargetActor_ContinuousTrace::Tick(float DeltaSeconds)
{
	if (SourceActor && SourceActor->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Perform the trace.
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);

		// Produce target data from any hits this frame.
		if (ShouldProduceTargetData() && OwningAbility)
		{
			TArray<FHitResult> FilteredHitResults;

			// Filter valid hits.
			for (FHitResult& Hit : HitResults)
			{
				MoveTemp(Hit);

				const AActor* HitActor = Hit.GetActor();
				
				// Ignore hits that didn't hit an actor.
				if (!HitActor)
				{
					continue;
				}
				
				// Don't hit the same target multiple times in a single targeting scope.
				if (HitTargets.Contains(HitActor))
				{
					continue;
				}

				// Perform target data filtering if we have a filter.
				if (Filter.Filter.IsValid() && !Filter.FilterPassesForActor(HitActor))
				{
					continue;
				}

				// Cache the hit actor so it doesn't repeat.
				HitTargets.Add(HitActor);

				// Some cues won't automatically trigger unless the hit is blocking.
				Hit.bBlockingHit = true;

				// Set the hit result's location for VFX.
				OverrideHitLocation(Hit);

				// Successful hit.
				FilteredHitResults.Add(Hit);
			}

			// Send all of this frame's hits together.
			TargetDataReadyDelegate.Broadcast(StartLocation.MakeTargetDataHandleFromHitResults(OwningAbility, FilteredHitResults));
		}
	}
}

void AGameplayAbilityTargetActor_ContinuousTrace::OverrideHitLocation(FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	ACrashCharacter* CrashCharacter = IsValid(HitActor) ? Cast<ACrashCharacter>(HitActor) : nullptr;
	USkeletalMeshComponent* TargetMesh = IsValid(CrashCharacter) ? CrashCharacter->GetThirdPersonMesh() : nullptr;
	if (!IsValid(TargetMesh))
	{
		return;
	}

	/* If we're aiming at the actor we hit, spawn FX where we were aiming. We don't use thin traces for hit detection
	 * (we're usually using big cones or spheres, since our melee attacks are pretty generous), so the raw hit result
	 * from the target data won't usually be visually accurate. */
	FHitResult HitForMesh;
	FCollisionShape SphereShape;
	SphereShape.SetSphere(10.0f);
	if (TargetMesh->SweepComponent(HitForMesh, HitResult.TraceStart, HitResult.TraceEnd, FQuat::Identity, SphereShape, false))
	{
		// Nudge the location back a little bit so it doesn't clip into the character mesh.
		const FVector Offset = (HitResult.Normal * -15.0f);

		HitResult.ImpactPoint = (HitForMesh.ImpactPoint + Offset);
		HitResult.ImpactNormal = HitForMesh.ImpactNormal;

		return;
	}


	// If weren't aiming right at the target, spawn VFX at the closest bone to where we're aiming.

	// Bones that we can spawn VFX at.
	TArray<FName> ValidHitBones = {
		"head",
		"spine_02",
		"lowerarm_l",
		"lowerarm_r",
		"pelvis",
		"calf_r",
		"calf_l"
	};

	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	if (APlayerController* PC = ActorInfo ? ActorInfo->PlayerController.Get() : nullptr)
	{
		FVector ViewLoc;
		FRotator ViewRot;
		PC->GetPlayerViewPoint(ViewLoc, ViewRot);
	
		FVector ClosestBoneLoc = FVector();
		float ClosestBoneDot = MIN_DOT_FOR_LOC_OVERRIDE; // We want the bone whose dot product with our LOS is closest to 1.0.
	
		FVector ClosestBackupBoneLoc = FVector();
		float ClosestBackupBoneDist = UE_BIG_NUMBER; // We want the bone closest to our trace origin (our camera).
	
		for (FName BoneName : ValidHitBones)
		{
			// Update preferred location.
			const FVector BoneLoc = TargetMesh->GetBoneLocation(BoneName);
			const FVector ViewNormal = (BoneLoc - HitResult.TraceStart).GetSafeNormal();
			const float BoneDot = ViewRot.Vector().Dot(ViewNormal);
	
			if (BoneDot > ClosestBoneDot)
			{
				ClosestBoneLoc = BoneLoc;
				ClosestBoneDot = BoneDot;
			}
	
			// Update backup location.
			const float Dist = FVector::Dist(BoneLoc, HitResult.TraceStart);
			if (Dist < ClosestBackupBoneDist)
			{
				ClosestBackupBoneLoc = BoneLoc;
				ClosestBackupBoneDist = Dist;
			}
		}
	
		/* If we use the bone's raw location, we'll spawn VFX inside the character, so we offset the location a
		 * little bit towards the trace origin. */
		const FVector Offset = (HitResult.Normal * -15.0f);
	
		/* If we're looking close enough at a certain bone, we'll override our hit's location to spawn VFX at that
		 * bone. */
		if (ClosestBoneDot > MIN_DOT_FOR_LOC_OVERRIDE)
		{
			HitResult.ImpactPoint = (ClosestBoneLoc + Offset);
		}
		/* If we're not looking close enough to any bones (e.g. we hit someone behind us), fall back to whichever
		 * bone is closest to our trace's origin (i.e. our camera). */
		else
		{
			HitResult.ImpactPoint = (ClosestBackupBoneLoc + Offset);
		}
	}
}
