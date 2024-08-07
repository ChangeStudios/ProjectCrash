// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagStack.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemDefinition;

class FLifetimeProperty;
struct FFrame;

/**
 * Represents an instance of an item that exists in the game. Item instances are
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UInventoryItemInstance();



	// Replication.

public:

	/** Enables replication. */
	virtual bool IsSupportedForNetworking() const override { return true; }

private:

#if UE_WITH_IRIS
	/** Registers this object's replication fragments. */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS



	// Item definition.

public:

	/** Getter for the item of which this object is an instance. */
	TSubclassOf<UInventoryItemDefinition> GetItemDefinition() const { return ItemDefinition; }

private:

	/** The item of which this object is an instance. */
	UPROPERTY(Replicated)
	TSubclassOf<UInventoryItemDefinition> ItemDefinition;



	// Traits.

public:

	// TODO: FindTraitByClass



	// Stat tags.

public:

	/** Adds the specified number of tags to this item's stat tags. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void AddStatTags(FGameplayTag Tag, int32 Count);

	/** Removes the specified number of tags from this item's stat tags. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveStatTags(FGameplayTag Tag, int32 Count);

	/** Returns how many of the specified tag this item has. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetStatTagCount(FGameplayTag Tag) const;

	/** Returns whether this item has at least one of the specified tag. Tag must match exactly. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasStatTag(FGameplayTag Tag) const;

private:

	/** A collection of tag stacks defining this item's stats. E.g. current ammo. */
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;



	// Ability system.

private:

	// friend ItemTrait_AbilitySystem

	/**
	 * Handles to any ability sets currently granted to this item's outer object by the item. These sets were granted
	 * when this item was added to the inventory, and will be removed when the item is removed. Only populated on the
	 * server.
	 *
	 * This is contained in the item instance rather than the ability system trait because traits cannot hold runtime
	 * data. This is a known limitation of the current inventory system.
	 */
	TArray<FCrashAbilitySet_GrantedHandles> GrantedAbilitySets;
};
