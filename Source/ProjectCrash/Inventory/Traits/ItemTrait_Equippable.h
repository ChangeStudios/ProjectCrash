// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Traits/InventoryItemTraitBase.h"
#include "ItemTrait_Equippable.generated.h"

class UEquipmentComponent;
class UEquipmentDefinition;

/**
 * This item can be "equipped" by pawns. Pawns must have an EquipmentComponent to equip items.
 */
UCLASS(DisplayName = "Equippable")
class PROJECTCRASH_API UItemTrait_Equippable : public UInventoryItemTraitBase
{
	GENERATED_BODY()

	friend UEquipmentComponent;

public:

	/** Automatically equips this item when it enters a pawn's inventory if AutoEquip is true. */
	virtual void OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const override;

	/** Unequips the equipment when the underlying item leaves the pawn's inventory. */
	virtual void OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const override;

protected:

	/** This item's static equipment data. Defines the equipment's appearance, runtime data class, etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEquipmentDefinition> EquipmentDefinition;

	/** If true, this item will automatically be equipped when it enters a player's inventory, or when the player loses
	 * their current equipment. Useful for intrinsic character equipment that should not be manually equipped. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", DisplayName = "Auto-Equip?")
	bool bAutoEquip = false;
};
