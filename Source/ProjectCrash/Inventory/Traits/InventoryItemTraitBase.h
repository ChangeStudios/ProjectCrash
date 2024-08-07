// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "InventoryItemTraitBase.generated.h"

class UInventoryItemInstance;

/**
 * A trait used to define the properties of an inventory item. Items are defined by a collection of traits in their
 * item definition, and only contain the traits relevant to the item. Traits can only contain static data and can be
 * accessed at any time via an item definition CDO.
 *
 * This base class should be subclassed into new traits to define new properties and/or logic as needed, rather than
 * expanding or modifying the item definition class itself.
 *
 * TODO: See if we can instantiate this per item instance so it can hold runtime data. Currently, item definitions are
 * never instantiated, so traits are only handled as CDOs. The only trait that actually needs runtime data is the
 * ability system trait. Our current workaround for this is storing that runtime data in the item instance class itself,
 * which is fine, but it may be better to keep trait-specific data in the traits themselves. 
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class PROJECTCRASH_API UInventoryItemTraitBase : public UObject
{
	GENERATED_BODY()

	/* These functions are called for each instance of any item definition with this trait. These functions behave like
	 * static functions: they are called on the item trait's CDO, since item traits are never instantiated. */

public:

	/** Called when a new item instance is spawned. */
	virtual void OnItemCreated(UInventoryItemInstance* ItemInstance) const {}

	/** Called when an item instance is destroyed. */
	virtual void OnItemDestroyed(UInventoryItemInstance* ItemInstance) const {}

	/** Called when an item instance is added to an inventory. NOT called when something like a "pick-up actor" takes
	 * ownership of an item. */
	virtual void OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const {}

	/** Called when an item instance is removed from an inventory. The item may have been destroyed, or it may have been
	 * dropped. */
	virtual void OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const {}

	
};
