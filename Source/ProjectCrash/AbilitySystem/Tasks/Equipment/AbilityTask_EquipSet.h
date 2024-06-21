// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Equipment/EquipmentSetDefinition.h"
#include "AbilityTask_EquipSet.generated.h"

class UEquipmentComponent;

/**
 * Predictively equips a given equipment set through the owner's equipment component.
 *
 * This task works by locally unequipping the current set and equipping the new set. The client then waits for
 * validation from the server to update the replicated equipment set variables (EquippedSet and TemporarilyEquippedSet).
 * If the server updates the variables to match what the client predictively set them to, then we know our prediction
 * was correct and our client is synced. If the variables don't match, our prediction missed, and we have to re-sync the
 * client with the server.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_EquipSet : public UAbilityTask
{
	GENERATED_BODY()

	// Task initialization.

public:

	/** Temporarily equips the given set. Equips the set predictively on the owning client. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", Meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_EquipSet* EquipSet
	(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UEquipmentSetDefinition* SetToEquip,
		bool bEquipTemporarily
	);



	// Task logic.

public:

	/** Predictively unequips the set equipped by this task, if the set was temporarily equipped. Non-temporary equipped
	 * sets cannot be directly unequipped. Note that this will end this task. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks")
	void UnequipTemporarySet();

protected:

	/** Performs predictive equipping. */
	virtual void Activate() override;

	/** Ensures client and server are in sync when this task ends. */
	virtual void OnDestroy(bool bInOwnerFinished) override;



	// Task parameters.

protected:

	/** The set being predictively equipped. */
	UPROPERTY()
	UEquipmentSetDefinition* SetToEquip;

	/** Whether the set is being equipped temporarily or as a full set. */
	bool bEquipTemporarily;



	// Internals.

private:

	/** The equipment component temporarily equipping the set. Cached for convenience. */
	UPROPERTY()
	UEquipmentComponent* EquipmentComponent;

	/**
	 * If this task's outer ability fails, we revert our predictive equip.
	 *
	 * Note: I'm not sure if this works; it's never executing when the outer ability fails, but the prediction is still
	 * getting correctly reverted somehow.
	 */
	UFUNCTION()
	void OnAbilityFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& Reason);

	/** Returns whether this is the instance of this task being run on the server or the locally predicted client. */
	FORCEINLINE bool IsServerTask() const;
};
