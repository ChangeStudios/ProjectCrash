// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryComponent.h"

#include "CrashStatics.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Register each item instance in this inventory as a replicated sub-object.
	for (FInventoryListEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	/* Register existing item instances as replicated sub-objects. This is needed to register item instances as
	 * replicated sub-objects if they were added to the inventory before this inventory component began replicating. */
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryListEntry& Entry : InventoryList.Entries)
		{
			UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

bool UInventoryComponent::CanAddItem(TSubclassOf<UInventoryItemDefinition> ItemDefinition)
{
	// TODO: Check for item restrictions (uniqueness, count, etc.).

	return true;
}

UInventoryItemInstance* UInventoryComponent::AddItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDefinition)
{
	UInventoryItemInstance* NewItem = nullptr;

	if (ItemDefinition != nullptr)
	{
		// Create a new instance for the given item through the inventory list.
		NewItem = InventoryList.AddEntry(ItemDefinition);

		// Replicate the new item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && NewItem)
		{
			AddReplicatedSubObject(NewItem);
		}
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to add an item without a valid definition to inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}

	return NewItem;
}

void UInventoryComponent::AddItemByInstance(UInventoryItemInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		// Add the item instance to the inventory.
		InventoryList.AddEntry(ItemInstance);

		// Start replicating the item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to add an invalid item to inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}
}

void UInventoryComponent::RemoveItem(UInventoryItemInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		// Stop replicating the item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}

		// Remove the item instance from the inventory.
		InventoryList.RemoveEntry(ItemInstance);
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to remove a null item from inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}
}

void UInventoryComponent::ClearInventory()
{
	// Remove each item in this inventory.
	for (auto EntryIt = InventoryList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryListEntry& Entry = *EntryIt;

		if (Entry.ItemInstance != nullptr)
		{
			RemoveItem(Entry.ItemInstance);
		}
	}
}

TArray<UInventoryItemInstance*> UInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UInventoryItemInstance* UInventoryComponent::FindFirstItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemToFind) const
{
	for (const FInventoryListEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

		if (IsValid(ItemInstance))
		{
			if (ItemInstance->GetItemDefinition() == ItemToFind)
			{
				return ItemInstance;
			}
		}
	}

	// An instance of the specified item was not found in this inventory.
	return nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryList);
}
