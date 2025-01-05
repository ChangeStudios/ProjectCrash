// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_ContinuousTrace.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"

// Number of traces to perform per second.
#define TRACE_RATE 30.0f

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

				// Successful hit.
				FilteredHitResults.Add(Hit);
			}

			// Send all of this frame's hits together.
			TargetDataReadyDelegate.Broadcast(StartLocation.MakeTargetDataHandleFromHitResults(OwningAbility, FilteredHitResults));
		}
	}
}
