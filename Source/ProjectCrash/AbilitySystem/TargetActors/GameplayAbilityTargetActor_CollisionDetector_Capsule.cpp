// Copyright Samuel Reitich 2024.


#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector_Capsule.h"

#include "Components/CapsuleComponent.h"

AGameplayAbilityTargetActor_CollisionDetector_Capsule::AGameplayAbilityTargetActor_CollisionDetector_Capsule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CapsuleRadius = 22.0f;
	CapsuleHalfHeight = 44.0f;

	// Create and cache this target actor's collision detection component.
	CollisionDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	DetectorAsCapsule = CollisionDetector ? Cast<UCapsuleComponent>(CollisionDetector) : nullptr;
}

void AGameplayAbilityTargetActor_CollisionDetector_Capsule::Configure(float InCapsuleRadius, float InCapsuleHalfHeight, bool bInRepeatTargets, TSubclassOf<AActor> InClassFilter, bool bInFilterForGASActors)
{
	check(DetectorAsCapsule);

	// Update this target actor's parameters.
	CapsuleRadius = InCapsuleRadius;
	CapsuleHalfHeight = InCapsuleHalfHeight;
	bRepeatTargets = bInRepeatTargets;
	ClassFilter = InClassFilter;
	bFilterForGASActors = bInFilterForGASActors;

	DetectorAsCapsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
}
