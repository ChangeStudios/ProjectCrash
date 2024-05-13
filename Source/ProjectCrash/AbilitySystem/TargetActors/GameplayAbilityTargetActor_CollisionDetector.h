// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_CollisionDetector.generated.h"

/**
 * A collision shape that detects collision while active. This should be subclassed with custom shapes. This actor
 * generates target data directly on the server.
 */
UCLASS(Abstract, NotPlaceable)
class PROJECTCRASH_API AGameplayAbilityTargetActor_CollisionDetector : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer);



	// Targeting.

public:
	
	/** Binds callbacks to when another actor overlaps with this actor's collision detector. */
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	/** Stops targeting and cleans up this target actor. */
	virtual void StopTargeting();
	
	/** Fires cancellation notifies and stops targeting. */
	virtual void CancelTargeting() override;

	/** Empties the array of targets. If bRepeatsTargets is false, targets that are detected by this actor can not be
	 * detected again until this is called to reset cached targets. */
	UFUNCTION(BlueprintCallable)
	void ResetTargets() { Targets.Empty(); }

protected:

	/** Generates and sends target data when an actor overlaps with this actor's collision detector. */
	UFUNCTION()
	virtual void OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** The collision component used to detect collision with this actor. */
	TObjectPtr<UShapeComponent> CollisionDetector;



	// Parameters.

protected:

	/** Whether to ignore the owner of the gameplay ability utilizing this target actor when checking for targets. */
	bool bIgnoreSelf;
		
	/** Whether the same targets can be detected multiple times. If false, the Targets array must be explicitly cleared
	 * before a target can be detected again, after being sent the first time. */
	bool bRepeatTargets;

	/** Optional class by which to filter targets. */
	UPROPERTY()
	TSubclassOf<AActor> ClassFilter;

	/** Whether to filter for targets with an ability system component. */
	bool bFilterForGASActors;

	/** Tags that targets are not allowed to have. If a hit target has any of these tags, it will be thrown out. */
	UPROPERTY()
	FGameplayTagContainer IgnoredTargetTags;



	// Internals.

protected:

	/** Targets that have been sent to the server by this actor. If bRepeatTargets is false, targets that are already
	 * in this array when they are detected will be filtered out. */
	UPROPERTY()
	TArray<AActor*> Targets;
};