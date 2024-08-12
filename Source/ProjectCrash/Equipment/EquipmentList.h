// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AbilitySystem/Abilities/CrashAbilitySet.h"
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
#if WITH_GAMEPLAY_DEBUGGER
	friend FGameplayDebuggerCategory_Equipment;
#endif // WITH_GAMEPLAY_DEBUGGER
	friend UEquipmentComponent;

public:

	/** Default constructor. */
	FEquipmentListEntry()
	{}

	/** Formats this entry as "Equipment Instance (Equipment Definition)". */
	FString GetDebugString() const;

private:

	/** The equipment of which this entry is an instance. */
	UPROPERTY()
	TObjectPtr<UEquipmentDefinition> EquipmentDefinition;

	/** The actual equipment represented by this equipment entry. */
	UPROPERTY()
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;

	/** Handles for the ability sets currently granted by this equipment. Only valid on authority. */
	UPROPERTY(NotReplicated)
	FCrashAbilitySet_GrantedHandles GrantedAbilitySetHandles;
};



/**
 * A list of equipment currently equipped by a pawn.
 */
USTRUCT(BlueprintType)
struct FEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

#if WITH_GAMEPLAY_DEBUGGER
	friend FGameplayDebuggerCategory_Equipment;
#endif // WITH_GAMEPLAY_DEBUGGER
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