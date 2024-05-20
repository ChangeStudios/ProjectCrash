// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "EquipmentPieceActor.generated.h"

struct FEquipmentPiece;
class UEquipmentComponent;

/**
 * Visually represents a single piece of an equipment set (a sword, a shield, a gun, etc.). Contains a single mesh
 */
UCLASS(NotBlueprintable)
class PROJECTCRASH_API AEquipmentPieceActor : public AActor
{
	GENERATED_BODY()

protected:

	/** The mesh component visually representing this equipment piece. */
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	/** The equipment component through which the owning actor equipped this piece's equipment set. */
	UPROPERTY()
	TObjectPtr<UEquipmentComponent> OwningEquipmentComp;

	/** The equipment piece data used to spawn this equipment piece actor. */
	FEquipmentPiece* EquipmentPiece;

	/** The perspective in which this actor represents its equipment piece. Each piece spawns both a first- and a
	 * third-person actor. */
	FGameplayTag Perspective;
};
