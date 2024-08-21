// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "EquipmentList.generated.h"

class FGameplayDebuggerCategory_Equipment;
class UEquipmentComponent;
class UEquipmentDefinition;
class UEquipmentInstance;
struct FEquipmentList;

/**
 * A single piece or collection of equipment. Represents an equipment instance.
 */
USTRUCT(BlueprintType)
struct FEquipmentListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	friend FEquipmentList;
	friend UEquipmentComponent;

public:

	/** Default constructor. */
	FEquipmentListEntry()
	{}

	/** Formats this entry as "Equipment Instance (Equipment Definition)". */
	FString GetDebugString() const;

private:

	/** The equipped instance represented by this equipment entry. */
	UPROPERTY()
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;
};



/**
 * A list of equipment currently equipped by a pawn.
 *
 * NOTE: This class is not used. Our current equipment system only allows one item to be equipped at any given time.
 * This struct was created for an older equipment system that supported equipping multiple items simultaneously.
 *
 * This code has not been removed in case a more sophisticated equipment system is desired in the future; e.g. if we
 * wanted to add a survival crafting game mode... for some (insane) reason. In this project's current state, however,
 * our more simplified, streamlined system is preferable.
 */
USTRUCT(BlueprintType)
struct FEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend UEquipmentComponent;

	// Construction.

public:

	/** Default constructor. */
	FEquipmentList()
		: OwningComponent(nullptr)
	{}

	/** Constructor initializing this equipment's owning component. */
	FEquipmentList(UActorComponent* InOwningComponent)
		: OwningComponent(InOwningComponent)
	{}



	// Equipment management.

public:

	/** Spawns and equips a new instance of the given equipment. */
	UEquipmentInstance* AddEntry(UEquipmentDefinition* EquipmentDefinition);

	/** Unequips the specified equipment. */
	void RemoveEntry(UEquipmentInstance* EquipmentInstance);

	/** Returns all currently equipped equipment. */
	TArray<UEquipmentInstance*> GetAllEquipment() const;



	// Replication.

/* OnReps. These are only called on clients. The equipment component's EquipItem/UnequipItem functions handle
 * server-side logic. */
public:

	/** Fires the OnUnequipped event on clients when equipment is unequipped. */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	/** Fires the OnEquipped event on clients when equipment is equipped. */
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	/** Not used, but required for serializer to bind to these functions correctly. */
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) {}

// Serialization.
public:

	/** Performs data serialization on this serializer's items. */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FEquipmentListEntry, FEquipmentList>(Entries, DeltaParams, *this);
	}



	// Internals.

private:

	/** The list of equipped equipment. */
	UPROPERTY()
	TArray<FEquipmentListEntry> Entries;

	/** The actor component to which this equipment belongs. Should be of type EquipmentComponent. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwningComponent;
};



/**
 * Struct required for fast array serialization.
 */
template<>
struct TStructOpsTypeTraits<FEquipmentList> : public TStructOpsTypeTraitsBase2<FEquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};