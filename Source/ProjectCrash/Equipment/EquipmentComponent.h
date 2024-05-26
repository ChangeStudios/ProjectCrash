// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentSetDefinition.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UAbilityTask_EquipSet;
class UEquipmentSetDefinition;

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

	/** Unequips everything before destroying this component. */
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;



	// Equipment management.

public:

	/** Equips the given equipment set. If the equipping character already has a set equipped, that set will be
	 * unequipped before the new set is equipped. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void EquipSet(UEquipmentSetDefinition* SetToEquip);

	/**
	 * Temporarily equips the given equipment set on the server. If an equipment set was already temporarily equipped,
	 * that set will be unequipped before the new set is equipped.
	 *
	 * The temporarily equipped set will override the current EquippedSet until UnequipTemporarySet is called, or until
	 * the temporary set is replaced by another temporary set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void TemporarilyEquipSet(UEquipmentSetDefinition* SetToTemporarilyEquip);

	/**
	 * Unequips the current temporarily equipped set and equips EquippedSet on the server. This should NOT be called
	 * when switching temporarily equipped sets.
	 *
	 * @returns		Whether a set was unequipped. Is false if this function is called when there isn't a set
	 *				temporarily equipped.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	bool UnequipTemporarySet();

	/** Detaches all spawned equipment actors from their attached actor, turning them into physics actors. */
	void DetachEquipment();



	// Internals.

// Equipment management.
private:

	/**
	 * Internal logic for equipping a new equipment set.
	 *
	 * @param SetToEquip					The new set to equip.
	 * @param OutEquippedSet				Returned handle for the equipped set.
	 */
	void EquipSet_Internal(UEquipmentSetDefinition* SetToEquip, FEquipmentSetHandle& OutEquippedSet);

	/**
	 * Internal logic for unequipping an equipment set. Equipment sets cannot be unequipped directly, so this is only
	 * called as a result of new sets being equipped, replacing old ones.
	 *
	 * @param bUnequipTemporarily		Whether to fully unequip the set, or only unequip it temporarily. If true,
	 *									the set will only be visually unequipped, allowing its granted abilities and
	 *									effects to remain active.
	 */
	void UnequipSet_Internal(FEquipmentSetHandle& SetToUnequip, bool bUnequipTemporarily);

// Current equipment set.
private:

	/** Unequips the previously equipped set and equips the new set. This is also where we confirm equipment set
	 * predictions. */
	UFUNCTION()
	void OnRep_EquippedSet(UEquipmentSetDefinition* PreviouslyEquippedSet);

	/** The equipment set currently equipped by this character. Can be overridden by TemporarilyEquippedSet. */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedSet)
	TObjectPtr<UEquipmentSetDefinition> EquippedSet;

	/** Handle for the current equipment set. */
	FEquipmentSetHandle EquippedSetHandle;

// Temporarily equipment set.
private:

	/** Unequips the previous temporary set and equips the new temporary set. If no temporary set was previously
	 * equipped, the current EquippedSet is unequipped, to override it with the new temporary set. This is also where
	 * we can confirm temporary set predictions. */
	UFUNCTION()
	void OnRep_TemporarilyEquippedSet(UEquipmentSetDefinition* PreviouslyTemporarilyEquippedSet);

	/** An equipment set temporarily overriding EquippedSet. */
	UPROPERTY(ReplicatedUsing = OnRep_TemporarilyEquippedSet)
	TObjectPtr<UEquipmentSetDefinition> TemporarilyEquippedSet;

	/** Handle for the current temporary equipment set. */
	FEquipmentSetHandle TemporarilyEquippedSetHandle;



	// Prediction. Should only be used by equipment ability tasks.

private:

	/** Called if a prediction misses to destroy the predicted set, reset prediction data, and equip the correct
	 * equipment. Can be called even if prediction succeeds to ensure the client and server are synced. */
	UFUNCTION(Client, Reliable)
	void Client_EquipPredictionFailed(bool bTemporarySet);

	/** Handle for the set that is predictively equipped. Used to destroy the predicted set if the prediction fails. */
	FEquipmentSetHandle PredictedEquipmentSetHandle;

/* Predicted data. These act like prediction keys for predicting a set equips. When the server updates the equipped set,
 * if these match the updated set, then our prediction was successful. If they don't match, our prediction failed. */
private:

	/** When we predictively equip a set, this gets set to the predicted set. This gets reset to nullptr when the
	 * server catches up and the client re-syncs, since we aren't predicting at that point. */
	UPROPERTY()
	TObjectPtr<UEquipmentSetDefinition> PredictedEquippedSet = nullptr;

	/** When we predictively equip a temporary set, this gets set to the predicted set. This gets reset to nullptr when
	 * the server catches up and the client re-syncs, since we aren't predicting at that point. */
	UPROPERTY()
	TObjectPtr<UEquipmentSetDefinition> PredictedTemporarySet = nullptr;

	/** Since we unequip temporary sets by setting them to null, we don't know if PredictedTemporarySet is null because
	 * it predicted an unequip, or if it's null because there's not prediction. */
	bool bPredictedUnequip = false;



	// Utils.

public:

	/** Retrieves the given actor's EquipmentComponent, if it has one. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UEquipmentComponent>() : nullptr); }

	/** Checks if this component's owner has authority. */
	bool HasAuthority() const { return GetOwner() && GetOwner()->HasAuthority(); }

	/** Returns the currently equipped set. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UEquipmentSetDefinition* GetEquippedSet() const { return EquippedSet; }

	/** Returns the temporarily equipped set, if there is one. Otherwise, returns nullptr. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UEquipmentSetDefinition* GetTemporarilyEquippedSet() const { return TemporarilyEquippedSet ? TemporarilyEquippedSet : nullptr; }

	friend UAbilityTask_EquipSet;
};
