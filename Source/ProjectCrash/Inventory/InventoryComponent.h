// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "InventoryList.h"
#include "Components/ControllerComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "InventoryComponent.generated.h"

class UPawnData;
class UInventoryItemDefinition;
class UInventoryItemInstance;

/**
 * Provides the owning controller with an inventory of items which can be accessed and managed via this component. Can
 * be added to player controllers or AI controllers.
 */
UCLASS(BlueprintType, ClassGroup = "Gameplay", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UInventoryComponent : public UControllerComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Starts listening for pawn changes to add and remove pawn-specific inventory items. */
	virtual void InitializeComponent() override;

	/** Stops listening for pawn changes. */
	virtual void UninitializeComponent() override;

private:

	/** Called when the pawn controlled by this inventory's owner changes. Adds the pawn's default items (defined in
	 * their pawn data) when possessed, and removes them when the pawn is killed. */
	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** Fired when the pawn this inventory's owner is possessing is ready to add inventory items from its pawn data. */
	FActorInitStateChangedBPDelegate ReadyForItems;

	/** Adds the items in this inventory's owner's pawn's pawn data. */
	UFUNCTION()
	void AddPawnItems(const FActorInitStateChangedParams& Params);

	/** TODO: Removes the dying pawn's default items from this inventory. */
	UFUNCTION()
	void OnPawnDeath();

	/** Treats switching pawns as a death. */
	UFUNCTION()
	void OnPawnDataChanged(const UPawnData* OldPawnData, const UPawnData* NewPawnData);



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

	/** Items granted to this inventory by its current pawn. Only valid on authority. */
	UPROPERTY()
	TArray<TObjectPtr<UInventoryItemInstance>> PawnItems;



	// Utils.

public:

	/** Retrieves a list of all items currently in this inventory. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UInventoryItemInstance*> GetAllItems() const;

	/** Returns the first instance of the given item in this inventory. Returns null if none can be found. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryItemInstance* FindFirstItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemToFind) const;

	/** Retrieves the given controller's inventory component, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	static UInventoryComponent* FindInventoryComponent(const AController* Controller) { return Controller ? Controller->FindComponentByClass<UInventoryComponent>() : nullptr; }
};
