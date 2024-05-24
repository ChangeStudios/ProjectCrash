// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentComponent.h"
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

	// Construction.

public:

	/** Default constructor. */
	AEquipmentPieceActor();



	// Initialization.

private:

	/* Initializes this equipment piece actor to represent the given equipment piece in the given perspective. */
	void InitEquipmentPieceActor(const FEquipmentPiece* InEquipmentPiece, const UEquipmentComponent* InOwningEquipmentComponent, FGameplayTag InEquipmentPerspective);

	// Equipment component is the only thing that should ever call InitEquipmentPieceActor.
	friend UEquipmentComponent;



	// Components.

protected:

	/** The mesh component visually representing this equipment piece. */
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> MeshComponent;



	// Internals.

private:

	/** The equipment piece data used to spawn this equipment piece actor. */
	const FEquipmentPiece* EquipmentPiece;

	/** The equipment component through which the owning actor equipped this piece's equipment set. */
	UPROPERTY()
	TObjectPtr<UEquipmentComponent> OwningEquipmentComponent;

	/** The perspective in which this actor represents its equipment piece. Each piece spawns both a first- and a
	 * third-person actor. */
	FGameplayTag Perspective;
};
