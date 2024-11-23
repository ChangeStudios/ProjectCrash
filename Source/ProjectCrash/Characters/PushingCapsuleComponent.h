// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "PushingCapsuleComponent.generated.h"

/**
 * Capsule component that pushes away pawns it overlaps.
 *
 * The default capsule component will prevent the owner from moving into or through other pawns, completing stopping
 * the owner's movement. This component allows the owner to move through other pawns, and will push those pawns away.
 */
UCLASS(ClassGroup="Collision", EditInlineNew, HideCategories = (Object, LOD, Lighting, TextureStreaming), Meta = (DisplayName = "Pushing Capsule Collision", BlueprintSpawnableComponent), MinimalAPI)
class UPushingCapsuleComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UPushingCapsuleComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	void TickCollider();

protected:

	/** The strength at which this component will push away other pawns. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape, Meta = (ClampMin = "0", UIMin = "0"))
	float PushStrength;

	UFUNCTION()
	void OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY()
	TArray<TObjectPtr<APawn>> CollidingPawns;
};
