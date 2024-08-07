// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryItemDefinition.h"

#include "Traits/InventoryItemTraitBase.h"


UInventoryItemDefinition::UInventoryItemDefinition() :
	DisplayName(FText::GetEmpty()),
	DeathBehavior(EItemDeathBehavior::Destroyed)
{
}

const UInventoryItemTraitBase* UInventoryItemDefinition::FindTraitByClass(TSubclassOf<UInventoryItemTraitBase> TraitClass) const
{
	if (TraitClass != nullptr)
	{
		// Search for a trait of the given class.
		for (UInventoryItemTraitBase* Trait : Traits)
		{
			if (Trait && Trait->IsA(TraitClass))
			{
				return Trait;
			}
		}
	}

	// Trait could not be found.
	return nullptr;
}
