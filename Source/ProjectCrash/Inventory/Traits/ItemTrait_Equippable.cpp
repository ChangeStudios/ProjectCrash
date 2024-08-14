// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/Traits/ItemTrait_Equippable.h"

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/EquipmentInstance.h"
#include "GameFramework/CrashLogging.h"
#include "Inventory/InventoryItemInstance.h"

void UItemTrait_Equippable::OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const
{
	// Equip this item if it should auto-equip.
	if (bAutoEquip)
	{
		UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponentFromItem(ItemInstance);

		// Missing equipment component.
		if (EquipmentComp == nullptr)
		{
			EQUIPMENT_LOG(Warning, TEXT("Attempted to auto-equip equipment [%s] from item [%s], but owner, [%s], does not have an associated equipment component."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(ItemInstance), *GetNameSafe(ItemInstance->GetOwner()));
			return;
		}

		// Try to equip the item.
		EquipmentComp->EquipItem(EquipmentDefinition, ItemInstance);
	}
}

void UItemTrait_Equippable::OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const
{
	// Unequip this item if it's currently equipped.
	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponentFromItem(ItemInstance);

	// Missing equipment component.
	if (EquipmentComp == nullptr)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to unequip equipment [%s] from item [%s], but owner, [%s], does not have an associated equipment component."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(ItemInstance), *GetNameSafe(ItemInstance->GetOwner()));
		return;
	}

	// Check if this item is currently equipped.
	if (UEquipmentInstance* Equipment = EquipmentComp->GetEquipment())
	{
		if (Equipment->GetInstigator() == ItemInstance)
		{
			// Unequip the item.
			EquipmentComp->UnequipItem();
		}
	}
}
