// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_CollisionDetector.generated.h"

/**
 * A collision shape that detects collision while active. This should be subclassed with custom shapes. 
 */
UCLASS(Abstract, NotPlaceable)
class PROJECTCRASH_API AGameplayAbilityTargetActor_CollisionDetector : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:

	AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

protected:

	UFUNCTION()
	void OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// The collision component used to detect collision with this actor.
	TObjectPtr<UShapeComponent> CollisionDetector;
};