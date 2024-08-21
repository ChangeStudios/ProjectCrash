// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "InventoryItemDefinition.generated.h"

class UInventoryItemTraitBase;

/**
 * Defines an item's behavior when its owner dies or is destroyed. By default, items are destroyed when their owner
 * dies.
 */
UENUM(BlueprintType)
enum class EItemDeathBehavior : uint8
{
	// This item is removed from its current inventory and destroyed.
	Destroyed,
	// This item remains in its current inventory.
	Persistent,
	// This item is dropped. Item must have the "Droppable" trait.
	Dropped
};



/**
 * Defines an item and its static properties. In-game items are instantiated from this definition.
 *
 * Items are defined with a collection of item "traits." Each trait defines static properties of an item, such as
 * whether it can be equipped. Items only contain the traits relevant to them. For example, if an item cannot be
 * equipped, it will not have the "equippable" trait.
 */
UCLASS(Abstract, Blueprintable, Const)
class PROJECTCRASH_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UInventoryItemDefinition();



	// Global item data.

public:

	/** This item's user-facing name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	/** TODO: What happens to this item when its owner dies. For players, this is used when the pawn "dies," not when the
	 * item's actual owner (i.e. the player state) is destroyed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	EItemDeathBehavior DeathBehavior;



	// Traits.

public:

	/** Traits defining this item's properties. Should only contain traits relevant to this item (e.g. this item should
	 * not have the "equippable" trait if it cannot be equipped). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Traits", Instanced)
	TArray<TObjectPtr<UInventoryItemTraitBase>> Traits;

	/** Returns this item's trait of the specified class. Returns null if this item does not have the given trait. */
	const UInventoryItemTraitBase* FindTraitByClass(TSubclassOf<UInventoryItemTraitBase> TraitClass) const;



	// Validation.

public:

#if WITH_EDITOR

	/** Prevents multiple instances of the same trait class from being added to a single item. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

#endif // WITH_EDITOR
};
