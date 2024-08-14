// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "InventoryList.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UInventoryItemDefinition;
class UInventoryItemInstance;

/**
 * Provides the owning actor with an inventory of items which can be accessed and managed via this component.
 */
UCLASS(BlueprintType, ClassGroup = "Gameplay", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	// TODO: UninitializeComponent should activate destruction logic for all inventory items.



	/*
	 * Replication.
	 *
	 * Inventory items are replicated as sub-objects of the inventory component in which they currently reside. This
	 * means that when an inventory item is NOT in an inventory (e.g. dropped as a "pick-up" actor in the world), its
	 * data will not be replicated. The object holding the item will need to implement its own logic if it wants to
	 * continue replicating the item's data.
	 *
	 * Even though the item's data will not be actively replicated, it WILL keep its current data, so it can continue
	 * replication when placed into a new inventory. Items should never change their data when not in an inventory,
	 * even if they ARE being replicated.
	 */

public:

	/** Registers each item instance in this inventory as a replicated sub-object. */
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	/** Registers any item instances that were added to the inventory before this component began replicating as
	 * replicated sub-objects when replication starts. */
	virtual void ReadyForReplication() override;



	// Inventory management.

// Adding items.
public:

	/** Returns whether the given item could be added to this inventory. */
	UFUNCTION(BlueprintPure, DisplayName = "Can Add Item?", Category = "Inventory")
	bool CanAddItem
	(
		UPARAM(DisplayName = "Item To Add")
		TSubclassOf<UInventoryItemDefinition> ItemDefinition
	);

	/** Creates and adds a new instance of the given item to this inventory. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName = "Add Item (New)", Category = "Inventory")
	UInventoryItemInstance* AddItemByDefinition
	(
		UPARAM(DisplayName = "Item To Add")
		TSubclassOf<UInventoryItemDefinition> ItemDefinition
	);

	/** Adds an existing item instance to this inventory. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName = "Add Item (Existing)", Category = "Inventory")
	void AddItemByInstance
	(
		UPARAM(DisplayName = "Item To Add")
		UInventoryItemInstance* ItemInstance
	);

// Removing items.
public:

	/** Removes the specified item instance from this inventory. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveItem
	(
		UPARAM(DisplayName = "Item To Remove")
		UInventoryItemInstance* ItemInstance
	);

	/** Removes all items from this inventory. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void ClearInventory();



	// Internals.

private:

	/** The actual inventory. */
	UPROPERTY(Replicated)
	FInventoryList InventoryList;



	// Utils.

public:

	/** Retrieves a list of all items currently in this inventory. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UInventoryItemInstance*> GetAllItems() const;

	/** Returns the first instance of the given item in this inventory. Returns null if none can be found. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItemInstance* FindFirstItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemToFind) const;

	/** Retrieves the given actor's inventory component, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static UInventoryComponent* FindInventoryComponent(AActor* Actor);
};
