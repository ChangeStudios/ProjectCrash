// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "EquipmentInstance.generated.h"

enum class EEquipmentPerspective : uint8;
class AEquipmentActor;
struct FEquipmentSkinActorInfo;

/**
 * A piece or set of equipment that is currently equipped. This is the tangible representation of equipment in the
 * world. This object tracks runtime equipment data (e.g. weapon heat) and handles equipment logic. This can be
 * subclassed to add additional equipment-specific logic, runtime data, or static data (as opposed to extending
 * EquipmentDefinition).
 *
 * This object differs from the EquipmentActor, as EquipmentActors are purely cosmetic: they do not handle equipment
 * data or logic, besides visual effects. Additionally, multiple EquipmentActors are spawned for each equipment (e.g.
 * one for first-person, one for third-person), while only one EquipmentInstance is spawned per equipment.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UEquipmentInstance : public UObject
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UEquipmentInstance();



	// Replication.

public:

	/** Enables replication. */
	virtual bool IsSupportedForNetworking() const override { return true; }

private:

#if UE_WITH_IRIS // We currently don't use Iris, but this will be needed if we ever switch to it.
	/** Registers this object's replication fragments. */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS



	// Instigator.

public:

	/** Returns the object responsible for creating this equipment instance (usually its inventory item instance). */
	UFUNCTION(BlueprintPure, Category = "Equipment", Meta = (ToolTip = "The object responsible for creating this equipment instance. Usually this equipment's inventory item instance."))
	UObject* GetInstigator() const { return Instigator; }

	/** Sets this equipment instance's instigating object. */
	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

private:

	/** The object directly responsible for creating this equipment instance. This is usually the equipment's inventory
	 * item instance. */
	UPROPERTY(Replicated)
	TObjectPtr<UObject> Instigator;



	// Equipping pawn.

public:

	/** The pawn on which this equipment is currently equipped. Should always be this equipment's outer object. */
	UFUNCTION(BlueprintPure, Category= "Equipment")
	APawn* GetPawn() const;

	/** Typed getter for the pawn on which this equipment is currently equipped. Returns null if the pawn is not of the
	 * desired type. */
	UFUNCTION(BlueprintPure, Category= "Equipment", Meta = (DeterminesOutputType = "PawnType"))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;



	// Equipment logic.

public:

	/** Called when this equipment instance spawned and equipped to a pawn. */
	virtual void OnEquipped();

	/** Called when this equipment instance is unequipped by a pawn, likely immediately before its destruction. */
	virtual void OnUnequipped();

protected:

	/** Blueprint-implementable version of OnEquipped. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", DisplayName = "OnEquipped", Meta = (ToolTip = "Called when this equipment instance spawned and equipped to a pawn."))
	void K2_OnEquipped();

	/** Blueprint-implementable version of OnUnequipped. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", DisplayName = "OnUnequipped", Meta = (ToolTip = "Called when this equipment instance is unequipped by a pawn, likely immediately before its destruction."))
	void K2_OnUnequipped();



	// Equipment actors.

public:

	/** Retrieves every equipment actor currently spawned to represent this equipment. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<AEquipmentActor*> GetSpawnedActors() const { return SpawnedActors; }

	/** Spawns the given collection of equipment actors in the given perspective for this equipment. This should
	 * usually be called twice for each equipment: once to spawn first-person actors and once for third-person
	 * actors. */
	void SpawnEquipmentActors(const TArray<FEquipmentSkinActorInfo>& ActorsToSpawn, EEquipmentPerspective Perspective);

	/** Destroys all equipment actors currently spawned by this equipment. */
	void DestroyEquipmentActors();

private:

	/** Equipment actors currently spawned in the world to represent this equipment while it is equipped. */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AEquipmentActor>> SpawnedActors;



	// Utils.

public:

	/** Tries to retrieve the world through the outer object, if the outer object is an actor (which it should always
	 * be). */
	virtual UWorld* GetWorld() const override;
};
