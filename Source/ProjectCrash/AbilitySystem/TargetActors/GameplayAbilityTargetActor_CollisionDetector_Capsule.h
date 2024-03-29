// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector.h"
#include "GameplayAbilityTargetActor_CollisionDetector_Capsule.generated.h"

class UCapsuleComponent;
/**
 * A collision detector using a configurable capsule collision shape.
 */
UCLASS(NotPlaceable)
class PROJECTCRASH_API AGameplayAbilityTargetActor_CollisionDetector_Capsule : public AGameplayAbilityTargetActor_CollisionDetector
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Constructs the collision capsule. */
	AGameplayAbilityTargetActor_CollisionDetector_Capsule(const FObjectInitializer& ObjectInitializer);



	// Configuration.

public:

	/**
	 * Configures this actor with the given parameters. This should be used to initialize this target actor before it
	 * is used.
	 *
	 * @param InCapsuleRadius			This actor's detection capsule's radius.
	 * @param InCapsuleHalfHeight		This actor's detection capsule's half-height.
	 * @param bInRepeatTargets			Whether the same targets can be detected multiple times. If false, the Targets
	 *									array must be explicitly cleared before a target can be detected again, after
	 *									being sent the first time. 
	 * @param InClassFilter				Optional class by which to filter targets.
	 * @param bInFilterForGASActors		Whether to filter for targets with an ability system component.
	 */
	UFUNCTION(BlueprintCallable)
	void Configure
	(
		float InCapsuleRadius = 22.0f,
		float InCapsuleHalfHeight = 44.0f,
		bool bInRepeatTargets = false,
		TSubclassOf<AActor> InClassFilter = nullptr,
		bool bInFilterForGASActors = true
	);



	// Parameters.

protected:

	float CapsuleRadius;

	float CapsuleHalfHeight;



	// Internals.

private:

	/** This actor's collision detector cast to a capsule component for convenience. */
	TObjectPtr<UCapsuleComponent> DetectorAsCapsule;
};
