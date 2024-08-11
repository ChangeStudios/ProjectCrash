// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "EquipmentInstance.generated.h"

class AEquipmentActor;
struct FEquipmentActorSpawnData;

/**
 * A piece or set of equipment that is currently equipped. This is the tangible representation of equipment in the
 * world. This object tracks runtime equipment data (e.g. weapon heat) and handles equipment logic. This can be
 * subclassed to add additional equipment-specific logic, runtime data, or static data (as opposed to extending
 * EquipmentDefinition).
 *
 * This object differs from the EquipmentActor, as EquipmentActors are purely cosmetic: they do not handle equipment
 * data or logic. Additionally, multiple EquipmentActors are spawned for each equipment (e.g. one for first-person, one
 * for third-person), while only one EquipmentInstance is spawned per equipment.
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

	UFUNCTION(BlueprintPure, Category = "Equipment")
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

private:

	UPROPERTY(Replicated)
	TObjectPtr<UObject> Instigator;



	// Equipping pawn.

public:

	UFUNCTION(BlueprintPure, Category=Equipment)
	APawn* GetPawn() const;

	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;



	// Equipment logic.

public:

	virtual void OnEquipped();

	virtual void OnUnequipped();

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", DisplayName = "OnEquipped")
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment", DisplayName = "OnEquipped")
	void K2_OnUnequipped();



	// Equipment actors.

public:

	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<AEquipmentActor*> GetSpawnedActors() const { return SpawnedActors; }

	void SpawnEquipmentActors(const TArray<FEquipmentActorSpawnData>& ActorsToSpawn, bool bSpawnInFirstPerson);

	void DestroyEquipmentActors();

private:

	UPROPERTY(Replicated)
	TArray<TObjectPtr<AEquipmentActor>> SpawnedActors;



	// Utils.

public:

	/** Tries to retrieve the world through the outer object, if the outer object is a pawn (which it should always
	 * be). */
	virtual UWorld* GetWorld() const override;
};
