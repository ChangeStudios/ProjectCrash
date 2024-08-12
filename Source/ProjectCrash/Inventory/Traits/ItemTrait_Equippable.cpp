// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/Traits/ItemTrait_Equippable.h"

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/EquipmentInstance.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/InventoryItemInstance.h"

void UItemTrait_Equippable::OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const
{
	// Equip this item if it should auto-equip.
	if (bAutoEquip)
	{
		UEquipmentComponent* EquipmentComp = FindEquipmentCompFromItem(ItemInstance);

		// Missing equipment component.
		if (EquipmentComp == nullptr)
		{
			EQUIPMENT_LOG(Error, TEXT("Attempted to auto-equip equipment [%s] from item [%s], but owner, [%s], does not have an associated equipment component."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(ItemInstance), *GetNameSafe(ItemInstance->GetOwner()));
			return;
		}

		// Try to equip the item.
		EquipmentComp->EquipItem(EquipmentDefinition);
	}
}

void UItemTrait_Equippable::OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const
{
	// Unequip this item if it's currently equipped.
	UEquipmentComponent* EquipmentComp = FindEquipmentCompFromItem(ItemInstance);

	// Missing equipment component.
	if (EquipmentComp == nullptr)
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to unequip equipment [%s] from item [%s], but owner, [%s], does not have an associated equipment component."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(ItemInstance), *GetNameSafe(ItemInstance->GetOwner()));
		return;
	}

	// Check if this item is currently equipped.
	for (UEquipmentInstance* Equipment : EquipmentComp->GetAllEquipment())
	{
		if (Equipment->GetInstigator() == ItemInstance)
		{
			// Unequip the item.
			EquipmentComp->UnequipItem(Equipment);
		}
	}
}

UEquipmentComponent* UItemTrait_Equippable::FindEquipmentCompFromItem(UInventoryItemInstance* ItemInstance)
{
	UEquipmentComponent* EquipmentComp = nullptr;

	AActor* Owner = ItemInstance->GetOwner();
	if (ensure(Owner))
	{
		// Get equipment component if item owner is a player state.
		if (APlayerState* OwnerAsPS = Cast<APlayerState>(Owner))
		{
			if (APawn* OwnerPawn = OwnerAsPS->GetPawn())
			{
				EquipmentComp = UEquipmentComponent::FindEquipmentComponent(OwnerPawn);
			}
		}

		// Get equipment component if item owner is a pawn.
		if (EquipmentComp == nullptr)
		{
			if (APawn* OwnerAsPawn = Cast<APawn>(Owner))
			{
				EquipmentComp = UEquipmentComponent::FindEquipmentComponent(OwnerAsPawn);
			}
		}
	}

	return EquipmentComp;
}
