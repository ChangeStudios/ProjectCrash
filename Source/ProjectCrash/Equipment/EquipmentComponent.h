// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "EquipmentList.h"
#include "Components/PawnComponent.h"
#include "EquipmentComponent.generated.h"

class UEquipmentDefinition;
class UEquipmentInstance;

/**
 * Allows the owning pawn to equip equipment which can be accessed and managed via this component.
 *
 * The pawn's controller does not need an inventory component to use equipment; equipment is defined with different data
 * than inventory items. But they do need an inventory if they want to equip items (i.e. inventory items with the
 * "equippable" trait).
 */
UCLASS(BlueprintType, Const, ClassGroup = "Gameplay", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UEquipmentComponent : public UPawnComponent
{
	GENERATED_BODY()

	// Construction.

public:

	UEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Attempts to equip any inventory items with auto-equip enabled. */
	virtual void InitializeComponent() override;

	/** Unequips all equipment when this component is uninitialized. */
	virtual void UninitializeComponent() override;



	/* Replication. The current equipment instance is replicated as a sub-object of the equipment component from it is
	 * equipped. Because equipment only exists while equipped (spawned on equip, destroyed on unequip), equipment is
	 * managed and replicated by the same equipment component for its entire lifetime. */

public:

	/** Registers the equipment instance equipped by this component as a replicated sub-object. */
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	/** Registers the equipped equipment instance as a replicated sub-object when replication starts, if it was equipped
	 * before this component began replicating. */
	virtual void ReadyForReplication() override;



	// Equipment management.

public:

	/** Equips a specified item. Unequips the current item. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	UEquipmentInstance* EquipItem
	(
		UPARAM(DisplayName = "Item to Equip")
		UEquipmentDefinition* EquipmentDefinition,
		UObject* Instigator = nullptr
	);

	/** Unequips the current item, leaving the pawn in an "unarmed" state. Do not call this to equip a new item; call
	 * EquipItem instead, which automatically unequips the current item. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void UnequipItem();



	// Internals.

private:

	/** Current equipment. Only one item can be equipped at a time. When a new item is equipped, the previous item is
	 * automatically unequipped. */
	UPROPERTY(ReplicatedUsing = "OnRep_CurrentEquipment")
	UEquipmentInstance* CurrentEquipment;

	/** Replicates the OnEquipped function to clients. */
	UFUNCTION()
	void OnRep_CurrentEquipment(UEquipmentInstance* PreviousEquipment);

	/** Attempts to auto-equip an equippable item in the new controller's player state's inventory, if it has one. */
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);



	// Utils.

public:

	/** Returns the equipment instance currently equipped by this component's owning pawn. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UEquipmentInstance* GetEquipment() const { return CurrentEquipment; }

	/** Templated version of GetEquipment. */
	template <typename T>
	T* GetEquipment();

	/** Retrieves the given pawn's inventory component, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const APawn* Pawn) { return Pawn->FindComponentByClass<UEquipmentComponent>(); }

	/** Tries to retrieve an equipment component associated with the owner of a given item. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponentFromItem(UInventoryItemInstance* ItemInstance);

private:

	/** Searches for an equippable item in an inventory component associated with this equipment component's owner, and
	 * equips the first item with "auto-equip" enabled. Does nothing if an item is already equipped. */
	void AutoEquipFirstItem();
};
