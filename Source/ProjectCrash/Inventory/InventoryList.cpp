// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryList.h"

#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"

/**
 * FInventoryListEntry
 */
FString FInventoryListEntry::GetDebugString() const
{
    TSubclassOf<UInventoryItemDefinition> ItemDefinition;
	if (ItemInstance != nullptr)
	{
		// TODO: ItemDefinition = ItemInstance->GetItemDefinition();
	}

    return FString::Printf(TEXT("%s (%s)"), *GetNameSafe(ItemInstance), *GetNameSafe(ItemDefinition));
}



/**
 * FInventoryList
 */
UInventoryItemInstance* FInventoryList::AddEntry(TSubclassOf<UInventoryItemDefinition> ItemDefinition)
{
	UInventoryItemInstance* NewItem = nullptr;

	check(ItemDefinition != nullptr);
	check(OwningComponent);

	AActor* OwningActor = OwningComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// Add a new entry in this inventory for the new item.
	FInventoryListEntry& NewEntry = Entries.AddDefaulted_GetRef();

	// Create a new instance of the desired item.
	NewEntry.ItemInstance = NewObject<UInventoryItemInstance>(OwningComponent->GetOwner());
	// TODO: NewEntry.ItemInstance->SetItemDefinition(ItemDefinition)
	// TODO: Fragments -> ForEach -> OnInstanceCreated(NewEntry.Instasnce);
	NewItem = NewEntry.ItemInstance;

	// Mark the new entry for replication.
	MarkItemDirty(NewEntry);

	return NewItem;
}

void FInventoryList::AddEntry(UInventoryItemInstance* ItemInstance)
{
	check(ItemInstance != nullptr);
	check(OwningComponent);

	AActor* OwningActor = OwningComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// Add a new entry in this inventory for the given item.
	FInventoryListEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemInstance = ItemInstance;
	// NOTE: The item instance's outer object will not be the owning component's actor. This might cause ownership problems.

	// Mark the new entry for replication.
	MarkItemDirty(NewEntry);
}

void FInventoryList::RemoveEntry(UInventoryItemInstance* ItemInstance)
{
	/* Remove any entries with a matching item instance. We iterate through every entry for redundancy; there should
	 * never be more than one entry for the same item instance. */
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryListEntry& Entry = *EntryIt;
		if (Entry.ItemInstance == ItemInstance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

TArray<UInventoryItemInstance*> FInventoryList::GetAllItems() const
{
	// Alloc space for each item in this inventory.
	TArray<UInventoryItemInstance*> Items;
	Items.Reserve(Entries.Num());

	// Collect the item instances from each inventory entry.
	for (const FInventoryListEntry& Entry : Entries)
	{
		if (Entry.ItemInstance != nullptr)
		{
			Items.Add(Entry.ItemInstance);
		}
	}

	return Items;
}
