// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Components/PrimitiveComponent.h"
#include "GameplayAbilityTargetActor_CollisionDetector.generated.h"

/**
 * A collision shape that detects collision while active. This should be subclassed with custom shapes.
 *
 * Subclasses should implement a "Configure" class to initialize this actor's data, and to reconfigure the data between
 * uses. See GameplayAbilityTargetActor_CollisionDetector_Capsule for reference.
 *
 * Some notes on debugging:
 *		* Enable the "Debug" parameter when instantiating this class to draw a debug trace when a collision is detected.
 *		* Toggle the "ShowDebug AbilitySystem" command to draw the target actor itself.
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

	/** Empties the cached array of targets. If bRepeatsTargets is false, targets that are detected by this actor can
	 * not be detected again until this is called to reset cached targets. If bInResetTargetsOnStart is true, this is
	 * done automatically when StartTargeting is called. */
	UFUNCTION(BlueprintCallable)
	void ResetTargets() { Targets.Empty(); }

protected:

	/** Generates and sends target data when an actor overlaps with this actor's collision detector. */
	UFUNCTION()
	virtual void OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Draws target actor-specific debugging information when a successful collision is detected. */
	UFUNCTION()
	virtual void DrawCollisionDebug(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) PURE_VIRTUAL(AGameplayAbilityTargetActor_CollisionDetector, );

	/** The collision component used to detect collision with this actor. Must be constructed by subclasses. */
	TObjectPtr<UShapeComponent> CollisionDetector;



	// Parameters. Should be initialized by a "Configure" function.

protected:

	/** The collision profile to use for the detection capsule. */
	UPROPERTY(BlueprintReadOnly)
	FName CollisionProfile;

	/** Whether to ignore the avatar of the gameplay ability using this target actor when checking for targets. */
	UPROPERTY(BlueprintReadOnly)
	bool bIgnoreSelf;

	/** Whether the same targets can be detected multiple times. If false, the Targets array must be explicitly cleared
	 * before a target can be detected again, after being sent the first time. */
	UPROPERTY(BlueprintReadOnly)
	bool bRepeatTargets;

	/** If true, target data will be automatically reset each time targeting starts. */
	UPROPERTY(BlueprintReadOnly)
	bool bResetTargetsOnStart;

	/** Whether to filter for targets with an ability system component. */
	UPROPERTY(BlueprintReadOnly)
	bool bFilterForGASActors;

	/** Tags that targets are not allowed to have. If a hit target has any of these tags, it will be thrown out. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer IgnoredTargetTags;



	// Internals.

protected:

	/** Targets that have been sent to the server by this actor. If bRepeatTargets is false, targets that are already
	 * in this array when they are detected will be filtered out. */
	UPROPERTY()
	TArray<AActor*> Targets;
};