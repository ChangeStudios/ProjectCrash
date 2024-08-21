// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/Traits/ItemTrait_InitializeStatistics.h"

#include "Inventory/InventoryItemInstance.h"

void UItemTrait_InitializeStatistics::OnItemCreated(UInventoryItemInstance* ItemInstance) const
{
	// Add this trait's stats to the item instance.
	for (const auto& KVP : InitialItemStatistics)
	{
		ItemInstance->AddStatTags(KVP.Key, KVP.Value);
	}
}

int32 UItemTrait_InitializeStatistics::GetDefaultStatByTag(FGameplayTag Tag) const
{
	if (const int32* StatPtr = InitialItemStatistics.Find(Tag))
	{
		return *StatPtr;
	}

	// Specified stat tag is not defined in this trait.
	return 0;
}
