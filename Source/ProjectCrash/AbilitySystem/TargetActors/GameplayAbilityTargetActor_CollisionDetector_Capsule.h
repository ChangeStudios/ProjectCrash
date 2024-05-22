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

	/** Binds debug info to the ability system debugger. */
	virtual void BeginPlay() override;



	// Configuration.

public:

	/**
	 * Configures this actor with the given parameters. This should be used to initialize this target actor before it
	 * is used.
	 *
	 * @param InCapsuleRadius						This actor's detection capsule's radius.
	 * @param InCapsuleHalfHeight					This actor's detection capsule's half-height.
	 * @param bInIgnoreSelf							Ignores this target data actor's SourceActor if hit.
	 * @param bInRepeatTargets						Whether the same targets can be detected multiple times. If false,
	 *												the Targets array must be explicitly cleared before a target can be
	 *												detected again, after being sent the first time. This is done
	 *												automatically if bInResetTargetsOnStart is true.
	 * @param bInResetTargetsOnStart				Automatically resets the hit targets each time targeting starts.
	 * @param InFilter								Optional target data filter used on hit actors.
	 * @param bInFilterForGASActors					Whether to filter for targets with an ability system component.
	 * @param IgnoreTargetsWithTags					Ignore any targets with any of these tags.
	 * @param bInShouldProduceTargetDataOnServer	Whether the server should wait for target data to be produced on
	 *												the client or if it should produce the data directly.
	 */
	UFUNCTION(BlueprintCallable)
	void Configure
	(
		float InCapsuleRadius = 22.0f,
		float InCapsuleHalfHeight = 44.0f,
		bool bInIgnoreSelf = true,
		bool bInRepeatTargets = false,
		bool bInResetTargetsOnStart = true,
		FGameplayTargetDataFilterHandle InFilter = FGameplayTargetDataFilterHandle(),
		bool bInFilterForGASActors = true,
		FGameplayTagContainer IgnoreTargetsWithTags = FGameplayTagContainer(),
		bool bInShouldProduceTargetDataOnServer = true
	);



	// Parameters.

protected:

	float CapsuleRadius;

	float CapsuleHalfHeight;



	// Internals.

private:

	/** This actor's collision detector cast to a capsule component for convenience. */
	TObjectPtr<UCapsuleComponent> DetectorAsCapsule;



	// Debug.

public:

#if WITH_EDITOR
	// Draws debug info when ability system debugging is enabled.
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
#endif // WITH_EDITOR
};
