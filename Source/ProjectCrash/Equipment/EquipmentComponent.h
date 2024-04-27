// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentSet.h"
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

	/** Unequips the current equipment set when this component is destroyed. */
	virtual void OnUnregister() override;



	// Equipment management.

public:

	/** Returns the equipment set currently equipped by this character. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	const UEquipmentSet* GetCurrentEquipmentSet() const { return CurrentEquipmentSet; }

	/**
	 * Authoritatively equips the given equipment set. If this character already has an equipment set equipped, that
	 * equipment set will be unequipped before equipping the new set.
	 *
	 * This can be used to explicitly unequip an equipment set by passing a nullptr as SetToEquip.
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
	 * @param SetToEquip					The set to try to equip.
	 * @param bWasTemporarilyUnequipped		If the equipment set is being re-equipped after having been temporarily
	 *										unequipped. If true, the set's granted abilities will be re-enabled instead
	 *										of being granted again.
	 * 
	 * @return								Whether the new set was successfully equipped. Will return false if this
	 *										character already has an equipment set equipped.
	 */
	bool EquipSet_Internal(UEquipmentSet* SetToEquip, bool bWasTemporarilyUnequipped = false);

	/**
	 * Attempts to unequip this character's current equipment set using the current handle.
	 *
	 * @param bTemporarilyUnequip	If true, the equipment set will only be temporarily unequipped. This disables
	 *								the set's granted abilities instead of removing them completely.
	 *
	 * @returns						Whether an equipment set was successfully unequipped. Will return false if this
	 *								character doesn't have a current equipment set to unequip.
	 */
	bool UnequipSet_Internal(bool bTemporarilyUnequip = false);

	/**
	 * Callback bound to when the ASC registered with this component gains or loses a TemporarilyUnequipped gameplay
	 * tag. This should be used instead of manual equipment methods because it does not null CurrentEquipmentSet.
	 *
	 * This is bound on all machines.
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

	/** Unequips this character's previous equipment set and equips the new one. */
	UFUNCTION()
	void OnRep_CurrentEquipmentSet(UEquipmentSet* PreviousEquipmentSet);

	/** The equipment set currently equipped by this character. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentEquipmentSet)
	TObjectPtr<UEquipmentSet> CurrentEquipmentSet;

	/** The handle for the equipment set currently equipped by this character. */
	FEquipmentSetHandle CurrentEquipmentSetHandle;



	// Utils.

public:

	/** Retrieves the given actor's EquipmentComponent, if it has one. Otherwise, returns nullptr. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UEquipmentComponent>() : nullptr); };

	/** Returns all first-person and third-person equipment actors currently spawned by this component. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<AActor*> GetEquipmentActors();

};
