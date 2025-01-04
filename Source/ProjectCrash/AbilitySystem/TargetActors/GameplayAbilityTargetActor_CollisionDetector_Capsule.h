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



	// Parameters.

protected:

	/** The radius of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true"))
	float CapsuleRadius;

	/** The half-height of the capsule being used for collision detection. */
	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn = "true"))
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
