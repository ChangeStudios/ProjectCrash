// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Traits/InventoryItemTraitBase.h"
#include "ItemTrait_AddAbilities.generated.h"

class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;

/**
 * An attribute set class and optional data used to initialize it.
 */
USTRUCT(BlueprintType)
struct FAbilitiesItemTraitEntry_AttributeSet
{
	GENERATED_BODY()

	/** Attribute set to grant. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAttributeSet> AttributeSetClass;

	/** Data table with which to initialize the attribute set, if any (can be left unset). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> InitializationData;
};



/**
 * Grants abilities, applies gameplay effects, and adds attribute sets to this item's owner when it enters their
 * inventory. Abilities, effects, and attribute sets are removed when this item leaves their inventory.
 */
UCLASS(DisplayName = "Add Abilities")
class PROJECTCRASH_API UItemTrait_AddAbilities : public UInventoryItemTraitBase
{
	GENERATED_BODY()

public:

	/** Grants abilities, applies gameplay effects, and adds attribute sets to the item's new owner. */
	virtual void OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const override;

	/** Removes the abilities, effects, and attribute sets granted by this item when it leaves its owner's inventory. */
	virtual void OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const override;
	
protected:

	/** Abilities that will be granted to this item's owner when it enters their inventory and removed when it leaves
	 * their inventory. */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", DisplayName = "Abilities to Grant")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	/** Effects that will be applied to this item's owner when it enters their inventory and removed when it leaves
	 * their inventory. */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", DisplayName = "Effects to Apply")
	TArray<TSubclassOf<UGameplayEffect>> Effects;

	/** Attribute sets that will be added to this item's owner when it enters their inventory and removed when it
	 * leaves their inventory. */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", DisplayName = "Attribute Sets to Add")
	TArray<FAbilitiesItemTraitEntry_AttributeSet> AttributeSets;
};
