// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/Traits/ItemTrait_Equippable.h"

void UItemTrait_Equippable::OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const
{
	if (bAutoEquip)
	{
		// TODO: Equip this item.
	}
}

void UItemTrait_Equippable::OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const
{
	// TODO: Unequip this item, if equipped.
}
