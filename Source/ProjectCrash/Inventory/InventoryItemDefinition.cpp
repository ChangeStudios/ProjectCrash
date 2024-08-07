// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryItemDefinition.h"

#include "Traits/InventoryItemTraitBase.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "Inventory"

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

#if WITH_EDITOR
EDataValidationResult UInventoryItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Search for multiple instances of any traits.
	TArray<TSubclassOf<UInventoryItemTraitBase>> SeenTraits;

	int32 Index = 0;
	for (UInventoryItemTraitBase* Trait : Traits)
	{
		if (Trait != nullptr)
		{
			TSubclassOf<UInventoryItemTraitBase> TraitClass = Trait->GetClass();

			if (SeenTraits.Contains(TraitClass))
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("RepeatedTrait", "Multiple instances of trait {0} in item {1}: index {2} and index {3}."), FText::FromString(GetNameSafe(TraitClass)), FText::FromString(GetNameSafe(this)), SeenTraits.Find(TraitClass), Index));
			}
			else
			{
				SeenTraits.Add(TraitClass);
			}
		}

		Index++;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE