// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryList.h"

#include "CrashGameplayTags.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Traits/InventoryItemTraitBase.h"

/**
 * FInventoryListEntry
 */
FString FInventoryListEntry::GetDebugString() const
{
    TSubclassOf<UInventoryItemDefinition> ItemDefinition;
	if (ItemInstance != nullptr)
	{
		ItemDefinition = ItemInstance->GetItemDefinition();
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
	NewEntry.ItemInstance = NewObject<UInventoryItemInstance>(OwningComponent->GetWorld()); // Outer object is the UWorld so this item doesn't get auto-destroyed when removed from its current inventory.
	NewEntry.ItemInstance->Init(OwningActor, ItemDefinition);

	// Notify the new item's traits that it has entered a new inventory.
	for (UInventoryItemTraitBase* Trait : GetDefault<UInventoryItemDefinition>(ItemDefinition)->Traits)
	{
		if (Trait != nullptr)
		{
			Trait->OnItemEnteredInventory(NewEntry.ItemInstance);
		}
	}

	// Broadcast an inventory change message on the server. Handled on clients by PostReplicatedAdd.
	BroadcastActionMessage(NewEntry, EInventoryChangeType::ItemAdded);

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

	// The item must have already been initialized with UInventoryItemInstance::Init.
	TSubclassOf<UInventoryItemDefinition> ItemDefinition = ItemInstance->GetItemDefinition();
	checkf(ItemDefinition, TEXT("Attempted to add uninitialized item [%s] to [%s]'s inventory."), *GetNameSafe(ItemInstance), *OwningActor->GetName());

	// Add a new entry in this inventory for the given item.
	FInventoryListEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.ItemInstance = ItemInstance;

	// Take ownership of the item.
	NewEntry.ItemInstance->SetOwner(OwningActor);

	// Notify the item's traits that it has entered a new inventory.
	for (UInventoryItemTraitBase* Trait : GetDefault<UInventoryItemDefinition>(ItemDefinition)->Traits)
	{
		if (Trait != nullptr)
		{
			Trait->OnItemEnteredInventory(NewEntry.ItemInstance);
		}
	}

	// Broadcast an inventory change message on the server. Handled on clients by PostReplicatedAdd.
	BroadcastActionMessage(NewEntry, EInventoryChangeType::ItemAdded);

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
			// Broadcast an inventory change message on the server. Handled on clients by PreReplicatedRemove.
			BroadcastActionMessage(Entry, EInventoryChangeType::ItemRemoved);

			// Notify the removed item's traits that it has left the inventory.
			if (TSubclassOf<UInventoryItemDefinition> ItemDefinition = Entry.ItemInstance->GetItemDefinition())
			{
				for (UInventoryItemTraitBase* Trait : GetDefault<UInventoryItemDefinition>(ItemDefinition)->Traits)
				{
					Trait->OnItemLeftInventory(Entry.ItemInstance);
				}
			}

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

void FInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// Broadcast a client-side inventory change message for each removed item.
	for (int32 Index : RemovedIndices)
	{
		FInventoryListEntry& Entry = Entries[Index];
		BroadcastActionMessage(Entry, EInventoryChangeType::ItemRemoved);
	}
}

void FInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// Broadcast a client-side inventory change message for each added item.
	for (int32 Index : AddedIndices)
	{
		FInventoryListEntry& Entry = Entries[Index];
		BroadcastActionMessage(Entry, EInventoryChangeType::ItemAdded);
	}
}

void FInventoryList::BroadcastActionMessage(FInventoryListEntry& Entry, EInventoryChangeType ChangeType)
{
	// Construct the message.
	FInventoryChangeMessage InventoryChangeMessage;
	InventoryChangeMessage.InventoryComponent = OwningComponent;
	InventoryChangeMessage.ItemInstance = Entry.ItemInstance;
	InventoryChangeMessage.ChangeType = ChangeType;

	// Broadcast the message (locally).
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwningComponent->GetWorld());
	MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_Inventory_Change, InventoryChangeMessage);
}
