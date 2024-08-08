// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryList.generated.h"

class UInventoryItemDefinition;
class UInventoryComponent;
class UInventoryItemInstance;
struct FInventoryList;

/**
 * A single entry in an inventory. Represents an item instance.
 */
USTRUCT(BlueprintType)
struct FInventoryListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	friend FInventoryList;
	friend UInventoryComponent;

public:

	/** Default constructor. */
	FInventoryListEntry()
	{}

	/** Formats this entry as "Item Instance (Item Definition)". */
	FString GetDebugString() const;

private:

	/** The actual item represented by this inventory entry. */
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> ItemInstance = nullptr;
};



/**
 * A list of inventory items. This list is the actual "inventory." Each entry represents a single item.
 */
USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend UInventoryComponent;

	// Construction.

public:

	/** Default constructor. */
	FInventoryList()
		: OwningComponent(nullptr)
	{}

	/** Constructor initializing this inventory's owning component. */
	FInventoryList(UActorComponent* InOwningComponent)
		: OwningComponent(InOwningComponent)
	{}



	// Inventory management.

public:

	/** Creates and adds a new instance of the given item to this inventory. */
	UInventoryItemInstance* AddEntry(TSubclassOf<UInventoryItemDefinition> ItemDefinition);

	/** Adds the given item instance to this inventory. */
	void AddEntry(UInventoryItemInstance* ItemInstance);

	/** Removes the specified item instance from this inventory. */
	void RemoveEntry(UInventoryItemInstance* ItemInstance);

	/** Returns each item instance in this inventory. */
	TArray<UInventoryItemInstance*> GetAllItems() const;



	// Replication.

public:

	/** Performs data serialization on this serializer's items. */
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryListEntry, FInventoryList>(Entries, DeltaParams, *this);
	}



	// Internals.

private:

	/** The contents of this inventory. This array is the atomic form of the inventory. */
	UPROPERTY()
	TArray<FInventoryListEntry> Entries;

	/** The actor component to which this inventory belongs. Usually of type InventoryComponent. */
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwningComponent;
};



/**
 * Struct required for fast array serialization.
 */
template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};