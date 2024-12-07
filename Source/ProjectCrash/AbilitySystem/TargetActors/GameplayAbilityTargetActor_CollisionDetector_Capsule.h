// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector.h"
#include "GameplayAbilityTargetActor_CollisionDetector_Capsule.generated.h"

class UCapsuleComponent;

/**
 * A collision detector using a configurable capsule collision shape.
 */
UCLASS(NotPlaceable, DisplayName = "Target Actor: Collision Detector, Capsule")
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
	 * Configures this actor with the given parameters. Must be called at least once to initialize this target actor
	 * before it is used. Can be used to reconfigure this actor's data between uses.
	 *
	 * @param InCapsuleRadius						This actor's detection capsule's radius.
	 * @param InCapsuleHalfHeight					This actor's detection capsule's half-height.
	 * @param InCollisionProfile					The collision profile to use for the detection capsule.
	 * @param bInRepeatTargets						Whether the same targets can be detected multiple times. If false,
	 *												the Targets array must be explicitly cleared before a target can be
	 *												detected again, after being sent the first time. This is done
	 *												automatically if bInResetTargetsOnStart is true.
	 * @param bInResetTargetsOnStart				Automatically resets the hit targets each time targeting starts.
	 * @param InFilter								Optional target data filter used on hit actors.
	 * @param IgnoreTargetsWithTags					Ignore any targets with any of these tags.
	 * @param bInShouldProduceTargetDataOnServer	If true, the server will generate the target data directly (or after
	 *												receiving a confirmation). If false, the server will wait for the
	 *												client to produce and send the data.
	 */
	UFUNCTION(BlueprintCallable)
	void Configure
	(
		UPARAM(DisplayName = "Capsule Radius")
		float InCapsuleRadius = 45.0f,
		UPARAM(DisplayName = "Capsule Half-Height")
		float InCapsuleHalfHeight = 90.0f,
		UPARAM(DisplayName = "Collision Profile", Meta = (GetOptions = "Engine.KismetSystemLibrary.GetCollisionProfileNames"))
		FName InCollisionProfile = FName("CapsuleHitDetection"),
		UPARAM(DisplayName = "Repeat Targets?")
		bool bInRepeatTargets = false,
		UPARAM(DisplayName = "Reset Targets on Start?")
		bool bInResetTargetsOnStart = true,
		UPARAM(DisplayName = "Filter")
		FGameplayTargetDataFilterHandle InFilter = FGameplayTargetDataFilterHandle(),
		FGameplayTagContainer IgnoreTargetsWithTags = FGameplayTagContainer(),
		UPARAM(DisplayName = "Produce Taget Data on Server?")
		bool bInShouldProduceTargetDataOnServer = true
	);



	// Parameters.

protected:

	/** The radius of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly)
	float CapsuleRadius;

	/** The half-height of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly)
	float CapsuleHalfHeight;



	// Internals.

protected:

	/** Draws debug info when a successful collision is detected. Draws a capsule where the collision occurred. */
	virtual void DrawCollisionDebug(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	/** This actor's collision detector cached as a capsule component for convenience. */
	TObjectPtr<UCapsuleComponent> DetectorAsCapsule;



	// Debugging.

public:

	/** Draws debug info when ability system debugging is enabled. Continuously draws a visual representation of this
	 * target actor. */
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
