// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentPieceDefinition.h"
#include "GameFramework/Actor.h"
#include "EquipmentActor.generated.h"

class UAbilitySystemComponent;
class UEquipmentComponent;
class UEquipmentPieceDefinition;

/**
 * Visually represents a single equipment piece. Can receive events for firing 
 */
UCLASS()
class PROJECTCRASH_API AEquipmentActor : public AActor
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AEquipmentActor();



	// Initialization.

public:

	/** Spawns this equipment actor's mesh. */
	void InitEquipmentActor(const UEquipmentComponent* InOwningEquipmentComponent, const UEquipmentPieceDefinition* InEquipmentPieceDefinition, ECharacterPerspective InEquipmentPerspective);

	/** Clears active effects */
	void OnUnequip();



	// Components.

protected:

	UPROPERTY()
	USceneComponent* Root = nullptr;

	/** The mesh representing this piece of equipment. */
	UPROPERTY()
	UMeshComponent* Mesh = nullptr;



	// Properties.

protected:

	/** The equipment component responsible for creating this equipment actor. */
	UPROPERTY()
	const UEquipmentComponent* OwningEquipmentComponent;

	/** The equipment piece that this actor is representing. */
	UPROPERTY()
	TObjectPtr<const UEquipmentPieceDefinition> SourceEquipmentPiece;

	/** Whether this actor represents its equipment piece in first-person or third-person. */
	ECharacterPerspective EquipmentPerspective;



	// Effects.

public:

	/** Sends an equipment event to this actor, which responds to it accordingly. */
	void HandleEquipmentEvent(FGameplayTag EventTag);

	/** Ends the specified equipment event for this equipment actor. */
	void EndEquipmentEvent(FGameplayTag EventTag);

protected:

	/** Active persistent cues playing on this actor. Cleared when this actor's source equipment set is unequipped. */
	UPROPERTY()
	TArray<FGameplayTag> ActiveEffectCues;



	// Utils.

private:

	/** Spawns and returns a mesh component with the given mesh. */
	void SpawnMeshComponent(UStreamableRenderAsset* InMesh, ECharacterPerspective InEquipmentPerspective);

	/** Retrieves any ASC owned by the owner of the given equipment component. */
	static UAbilitySystemComponent* GetASCFromEquipmentComponent(const UEquipmentComponent* InEquipmentComponent);
};
