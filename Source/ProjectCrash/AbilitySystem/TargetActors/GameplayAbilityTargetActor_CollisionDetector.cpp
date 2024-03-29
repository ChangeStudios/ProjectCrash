// Copyright Samuel Reitich 2024.


#include "GameplayAbilityTargetActor_CollisionDetector.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "Components/ShapeComponent.h"

AGameplayAbilityTargetActor_CollisionDetector::AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	ShouldProduceTargetDataOnServer = true;

	CollisionDetector = nullptr;

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
	CollisionDetector->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCollisionBegin);
}

void AGameplayAbilityTargetActor_CollisionDetector::OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldProduceTargetData())
	{
		// Perform optional class filtering.
		if (ClassFilter && OtherActor->GetClass() != ClassFilter)
		{
			return;
		}

		// Perform GAS filtering.
		if (bFilterForGASActors && !UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor))
		{
			return;
		}

		// Check if the actor has already been detected as a target.
		if (!bRepeatTargets && Targets.Contains(OtherActor))
		{
			return;
		}

		// Cache the target.
		Targets.Add(OtherActor);

		// Generate and send target data with the overlapped actor.
		const FGameplayAbilityTargetDataHandle TargetDataHandle = StartLocation.MakeTargetDataHandleFromActors(TArray<TWeakObjectPtr<AActor>>({  OtherActor }), true);
		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}
}
