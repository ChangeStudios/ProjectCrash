// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Inventory/Traits/InventoryItemTraitBase.h"
#include "GameplayTagContainer.h"
#include "ItemTrait_InitializeStatistics.generated.h"

class UInventoryItemInstance;

/**
 * Initializes this item's statistics with a collection of a specified number of gameplay tags. This is a good way to,
 * for example, initialize a weapon's ammo.
 */
UCLASS(DisplayName = "Initialize Item Statistics")
class PROJECTCRASH_API UItemTrait_InitializeStatistics : public UInventoryItemTraitBase
{
	GENERATED_BODY()

public:

	/** Initializes this item's stat tags. */
	virtual void OnItemCreated(UInventoryItemInstance* ItemInstance) const override;

	/** Retrieves the initial value of one of this item's statistics (rather than its current stat value). */
	int32 GetDefaultStatByTag(FGameplayTag Tag) const;

protected:

	/** The statistics with which this item will be initialized. */
	UPROPERTY(EditDefaultsOnly, Category = "Item Statistics")
	TMap<FGameplayTag, int32> InitialItemStatistics;
};
