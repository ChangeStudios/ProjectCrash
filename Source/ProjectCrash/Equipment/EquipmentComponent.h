// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "EquipmentComponent.generated.h"

class UEquipmentSet;

/**
 * Manages the owning character's equipment, acting as an interface to the equipment system. Can only be given to
 * character classes.
 */
UCLASS()
class PROJECTCRASH_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UEquipmentComponent();

protected:

	/** Ensures that this component is attached to a character class and is the only instance of this component. */
	virtual void OnRegister() override;



	// Initialization.

public:

	/** Binds the OnTemporarilyUnequippedChanged callback. */
	virtual void InitializeComponent() override;



	// Equipment management.

public:

	/** Returns the equipment set currently equipped by this character. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	const UEquipmentSet* GetCurrentEquipmentSet() const { return CurrentEquipmentSet; }

	/**
	 * Equips the given equipment set. If this character already has an equipment set equipped, that equipment set will
	 * be unequipped before equipping the new set.
	 *
	 * @param SetToEquip	The set to equip.
	 *
	 * @returns				The equipment set that was unequipped and replaced by the new set. Returns nullptr if this
	 *						character did not have an equipment set equipped.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	UEquipmentSet* EquipEquipmentSet(UEquipmentSet* SetToEquip);

private:

	/**
	 * Attempts to equip the given equipment set. Will not equip the set if this character already has an equipment
	 * set equipped.
	 * 
	 * @param SetToEquip	The set to try to equip.
	 * 
	 * @return				Whether the new set was successfully equipped. Will return false if this character already
	 *						has an equipment set equipped.
	 */
	bool EquipSet_Internal(UEquipmentSet* SetToEquip);

	/**
	 * Attempts to unequip this character's current equipment set.
	 *
	 * @returns		Whether an equipment set was successfully unequipped. Will return false if this character doesn't
	 *				have a current equipment set to unequip.
	 */
	bool UnequipSet_Internal();

	/**
	 * Callback bound to when the ASC registered with this component gains or loses a TemporarilyUnequipped gameplay
	 * tag.
	 * 
	 * @param Tag			TemporarilyUnequipped tag.
	 * @param NewCount		Number of TemporarilyUnequipped tags the ASC now has.
	 */
	UFUNCTION()
	void OnTemporarilyUnequippedChanged(const FGameplayTag Tag, int32 NewCount);

	/** Delegate to bind the OnTemporarilyUnequippedChanged callback. */
	FDelegateHandle TemporarilyUnequippedDelegate;



	// Equipment.

private:

	/** The equipment set currently equipped by this character. */
	TObjectPtr<UEquipmentSet> CurrentEquipmentSet;

};
