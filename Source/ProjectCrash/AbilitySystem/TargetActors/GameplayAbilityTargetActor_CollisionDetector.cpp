// Copyright Samuel Reitich 2024.


#include "GameplayAbilityTargetActor_CollisionDetector.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Components/ShapeComponent.h"

AGameplayAbilityTargetActor_CollisionDetector::AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	ShouldProduceTargetDataOnServer = true;

	CollisionDetector = nullptr;

	bIgnoreSelf = true;
	bRepeatTargets = false;
	ClassFilter = nullptr;
	bFilterForGASActors = true;

	Targets = TArray<AActor*>();
}

void AGameplayAbilityTargetActor_CollisionDetector::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	StartLocation.SourceActor = SourceActor;
	StartLocation.SourceAbility = Ability;
	StartLocation.LiteralTransform = IsValid(CollisionDetector) ? CollisionDetector->GetComponentTransform() : FTransform();

	/* Ensure the collision detector has been created. Subclasses of AGameplayAbilityTargetActor_CollisionDetector
	 * must create their own collision detector component. */
	ensureAlwaysMsgf(IsValid(CollisionDetector), TEXT("%s: CollisionDetector component has not been created. Subclasses of the AGameplayAbilityTargetActor_CollisionDetector class must create a CollisionDetector component to function properly."), *GetClass()->GetName());

	// Bind a callback to when another actor overlaps this collision component.
	if (!CollisionDetector->OnComponentBeginOverlap.IsAlreadyBound(this, &ThisClass::OnCollisionBegin))
	{
		CollisionDetector->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCollisionBegin);
	}

	// Perform an initial overlap check for any actors that were already overlapping this component.
	TArray<UPrimitiveComponent*> OutComponents;
	CollisionDetector->GetOverlappingComponents(OutComponents);
	for (UPrimitiveComponent* OverlappingComp : OutComponents)
	{
		OnCollisionBegin(CollisionDetector, OverlappingComp->GetOwner(), OverlappingComp, -1, false, FHitResult());
	}
}

void AGameplayAbilityTargetActor_CollisionDetector::StopTargeting()
{
	// Clear callbacks.
	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();
	CollisionDetector->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnCollisionBegin);

	if (GenericDelegateBoundASC)
	{
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor_CollisionDetector::CancelTargeting);
		GenericDelegateBoundASC = nullptr;
	}
}

void AGameplayAbilityTargetActor_CollisionDetector::CancelTargeting()
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
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor_CollisionDetector::CancelTargeting called with null ASC! Actor %s"), *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());

	// Clean up targeting actor.
	StopTargeting();
}

void AGameplayAbilityTargetActor_CollisionDetector::OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldProduceTargetData())
	{
		// Perform owner filtering.
		if (bIgnoreSelf && OtherActor == SourceActor)
		{
			return;
		}

		// Perform optional class filtering.
		if (ClassFilter && OtherActor->GetClass() != ClassFilter)
		{
			return;
		}

		// Perform GAS filtering.
		if (bFilterForGASActors)
		{
			const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
			if (!ASC || ASC->HasAnyMatchingGameplayTags(IgnoredTargetTags))
			{
				return;
			}
		}

		// Check if the actor has already been detected as a target.
		if (!bRepeatTargets && Targets.Contains(OtherActor))
		{
			return;
		}

		// Cache the target.
		Targets.Add(OtherActor);

		// Generate and send target data with the overlapped actor.
		TargetDataReadyDelegate.Broadcast(StartLocation.MakeTargetDataHandleFromActors( TArray<TWeakObjectPtr<AActor>>({OtherActor}) ));
	}
}
