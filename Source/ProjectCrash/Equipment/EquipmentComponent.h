// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentSetDefinition.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UEquipmentSet;

/**
 * Grants a character access to the equipment system. Acts as an interface between the character and the equipment
 * system, allowing them to equip and unequip equipment sets.
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

	/** Ensures this component is attached to a character class. */
	virtual void OnRegister() override;



	// Effects.

public:

	/** Sends the given equipment effect event to this component, which routes it to all active equipment actors. */
	UFUNCTION(BlueprintCallable, Category = "Equipment", Meta = (Categories = "Event.EquipmentEffect"))
	void SendEquipmentEffectEvent(FGameplayTag EffectEvent);



	// Equipment management.

public:

	/** Equips the given equipment set on the server. If the equipping character already has a set equipped, that set
	 * will be unequipped before the new set is equipped. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void EquipSet(UEquipmentSet* SetToEquip);

	/**
	 * Temporarily equips the given equipment set on the server. If an equipment set was already temporarily equipped,
	 * that set will be unequipped before the new set is equipped.
	 *
	 * The temporarily equipped set will override the current EquippedSet until UnequipTemporarySet is called, or until
	 * the temporary set is replaced by another temporary set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void TemporarilyEquipSet(UEquipmentSet* SetToTemporarilyEquip);

	/**
	 * Unequips the current temporarily equipped set and equips EquippedSet. This should NOT be called when switching
	 * temporarily equipped sets.
	 *
	 * @returns		Whether a set was unequipped. Is false if this function is called when there isn't a set
	 *				temporarily equipped.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	bool UnequipTemporarySet();



	// Internals.

// Equipment management.
private:

	/** Internal logic for equipping a new equipment set. Granting abilities, spawning actors, etc. */
	void EquipSet_Internal(UEquipmentSet* SetToEquip);

	/**
	 * Internal logic for unequipping an equipment set. Equipment sets cannot be unequipped directly, so this is only
	 * called as a result of new sets being equipped, replacing old ones.
	 *
	 * @param bUnequipTemporarySet		Whether to unequip TemporarilyEquippedSet or EquippedSet.
	 * @param bUnequipTemporarily		Whether to fully unequip the set, or only unequip it temporarily. If true,
	 *									the set will only be visually unequipped, allowing its granted abilities and
	 *									effects to remain active.
	 */
	void UnequipSet_Internal(bool bUnequipTemporarySet, bool bUnequipTemporarily);

// Current equipment set.
private:

	/** Unequips the previously equipped set and equips the new set. */
	UFUNCTION()
	void OnRep_EquippedSet(UEquipmentSet* PreviouslyEquippedSet);

	/** The equipment set currently equipped by this character. Can be overridden by TemporarilyEquippedSet. */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedSet)
	TObjectPtr<UEquipmentSet> EquippedSet;

	/** Handle for the current equipment set. */
	FEquipmentSetHandle EquippedSetHandle;

// Temporarily equipment set.
private:

	/** Unequips the previous temporary set and equips the new temporary set. If no temporary set was previously
	 * equipped, the current EquippedSet is unequipped, to override it with the new temporary set. */
	UFUNCTION()
	void OnRep_TemporarilyEquippedSet(UEquipmentSet* PreviouslyTemporarilyEquippedSet);

	/** An equipment set temporarily overriding EquippedSet. */
	UPROPERTY(ReplicatedUsing = OnRep_TemporarilyEquippedSet)
	TObjectPtr<UEquipmentSet> TemporarilyEquippedSet;

	/** Handle for the current temporary equipment set. */
	FEquipmentSetHandle TemporarilyEquippedSetHandle;



	// Utils.

public:

	/** Retrieves the given actor's EquipmentComponent, if it has one. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UEquipmentComponent>() : nullptr); }
};
