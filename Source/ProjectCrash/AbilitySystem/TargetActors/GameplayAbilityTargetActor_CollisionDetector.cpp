// Copyright Samuel Reitich. All rights reserved.


#include "GameplayAbilityTargetActor_CollisionDetector.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbility.h"
#include "CrashGameplayTags.h"
#include "Components/ShapeComponent.h"

AGameplayAbilityTargetActor_CollisionDetector::AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	/** Generate target data directly on the server after receiving confirmation from the client. Don't trust the data
	 * that the client produces. */
	ShouldProduceTargetDataOnServer = true;

	CollisionDetector = nullptr;

	CollisionProfile = FName("CapsuleHitDetection");
	bAttachToCharacter = false;
	bRepeatTargets = false;
	bResetTargetsOnStart = true;

	Targets = TArray<AActor*>();
}

void AGameplayAbilityTargetActor_CollisionDetector::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	StartLocation.SourceActor = SourceActor;
	StartLocation.SourceAbility = Ability;
	StartLocation.LiteralTransform = IsValid(CollisionDetector) ? CollisionDetector->GetComponentTransform() : FTransform();

	/* Check that the collision detector has been created. Subclasses of AGameplayAbilityTargetActor_CollisionDetector
	 * must construct their own collision detector component. */
	checkf(IsValid(CollisionDetector), TEXT("%s: CollisionDetector component has not been created. Subclasses of the AGameplayAbilityTargetActor_CollisionDetector class must create a CollisionDetector component to function properly."), *GetClass()->GetName());

	// Use the desired collision profile. This lets us configure what we want to detect (e.g. pawn meshes or capsules).
	CollisionDetector->SetCollisionProfileName(CollisionProfile.Name);

	// Attach this target actor to the owning character if desired.
	if (bAttachToCharacter)
	{
		if (AActor* Avatar = Ability->GetAvatarActorFromActorInfo())
		{
			AttachToActor(Avatar, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}

	// Reset the hit targets each time targeting restarts, if desired.
	if (bResetTargetsOnStart)
	{
		Targets.Empty();
	}

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
		GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(this, &ThisClass::ConfirmTargeting);
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &ThisClass::CancelTargeting);
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
	if (ShouldProduceTargetData() && ensure(OwningAbility))
	{
		// Perform target data filtering.
		if (Filter.Filter.IsValid() && !Filter.FilterPassesForActor(OtherActor))
		{
			return;
		}

		// Check if the actor has already been detected as a target.
		if (!bRepeatTargets && Targets.Contains(OtherActor))
		{
			return;
		}

		// Cache the target so it doesn't repeat.
		Targets.Add(OtherActor);

		/* Generate a hit result for the target data so it can be used for FX triggers. bBlockingHit must be true to
		 * trigger certain cues. */
		FHitResult TargetHit;
		const FVector OwningActorLoc = GetActorLocation();
		const FVector TargetActorLoc = OtherActor->GetActorLocation();
		const float SweepRadius = 50.0f;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(OwningAbility->GetAvatarActorFromActorInfo());
		GetWorld()->SweepSingleByChannel(TargetHit, OwningActorLoc, TargetActorLoc, FQuat::Identity, ECC_GameTraceChannel1 /** AbilityTarget */, FCollisionShape::MakeSphere(SweepRadius), CollisionParams);

		/* If the trace fails, generate an artificial hit result to ensure we at least have an approximation of the
		 * necessary data. */
		if (!TargetHit.bBlockingHit || (TargetHit.GetActor() != OtherActor))
		{
			TargetHit.bBlockingHit = true;
			TargetHit.HitObjectHandle = FActorInstanceHandle(OtherActor);
			TargetHit.Component = OtherComp;
			TargetHit.Location = TargetActorLoc;
			TargetHit.Normal = FVector(OwningActorLoc - TargetActorLoc).GetSafeNormal();
			TargetHit.ImpactPoint = TargetActorLoc;
			TargetHit.ImpactNormal = TargetHit.Normal;
			TargetHit.Distance = FVector::Dist(OwningActorLoc, TargetActorLoc);
		}

		// Generate and send the generated target data.
		TargetDataReadyDelegate.Broadcast(StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, TargetHit));

#if ENABLE_DRAW_DEBUG
		// Draw debug info on collision detection.
		if (bDebug)
		{
			DrawCollisionDebug(OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		}
#endif // ENABLE_DRAW_DEBUG
	}
}
