// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Tasks/Equipment/AbilityTask_EquipSet.h"

#include "AbilitySystemComponent.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/CrashLogging.h"


UAbilityTask_EquipSet* UAbilityTask_EquipSet::EquipSet(UGameplayAbility* OwningAbility, FName TaskInstanceName, UEquipmentSetDefinition* SetToEquip, bool bEquipTemporarily)
{
	// Create a new task.
	UAbilityTask_EquipSet* MyTask = NewAbilityTask<UAbilityTask_EquipSet>(OwningAbility, TaskInstanceName);

	// Cache this task's parameters.
	MyTask->SetToEquip = SetToEquip;
	MyTask->bEquipTemporarily = bEquipTemporarily;

	return MyTask;
}

void UAbilityTask_EquipSet::UnequipTemporarySet()
{
	if (EquipmentComponent && bEquipTemporarily)
	{
		// Unequip the temporary set, locally (predictively) or authoritatively.
		if (EquipmentComponent->UnequipTemporarySet())
		{
			// Cache the prediction on the client.
			if (!IsServerTask())
			{
				EquipmentComponent->bPredictedUnequip = true;

				/* The equipped set, which will be equipped when the temporary set is unequipped, is now the predicted
				 * set. */
				EquipmentComponent->PredictedEquippedSet = SetToEquip;
				EquipmentComponent->PredictedEquipmentSetHandle = FEquipmentSetHandle(EquipmentComponent->EquippedSetHandle);
			}
		}
	}

	// Note: May want to remove this if it messes with prediction.
	EndTask();
}

void UAbilityTask_EquipSet::Activate()
{
	Super::Activate();

	const bool bAuthority = IsServerTask();
	EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(Ability->GetAvatarActorFromActorInfo());
	if (!EquipmentComponent)
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to activate AbilityTask_EquipSet, but the ability's avatar [%s] does not have an equipment component!"), *GetNameSafe(Ability->GetAvatarActorFromActorInfo()));
		EndTask();
	}

	// Listen for this task's outer ability's failure. If the ability fails, we have to revert our predicted equip.
	if (!bAuthority && AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &UAbilityTask_EquipSet::OnAbilityFailed);
	}

	// Cleanse prediction data.
	if (!bAuthority)
	{
		EquipmentComponent->PredictedEquippedSet = nullptr;
		EquipmentComponent->PredictedTemporarySet = nullptr;
		EquipmentComponent->bPredictedUnequip = false;
	}

	// Equip the set, locally (predictively) or authoritatively.
	if (bEquipTemporarily)
	{
		EquipmentComponent->TemporarilyEquipSet(SetToEquip);
	}
	else
	{
		EquipmentComponent->EquipSet(SetToEquip);
	}

	// On the client, cache the predictively equipped set so it can be removed if the prediction fails.
	if (!bAuthority)
	{
		if (bEquipTemporarily)
		{
			EquipmentComponent->PredictedTemporarySet = SetToEquip;
		}
		else
		{
			EquipmentComponent->PredictedEquippedSet = SetToEquip;
		}

		EquipmentComponent->PredictedEquipmentSetHandle = FEquipmentSetHandle(bEquipTemporarily ? EquipmentComponent->EquippedSetHandle : EquipmentComponent->TemporarilyEquippedSetHandle);
	}
}

void UAbilityTask_EquipSet::OnDestroy(bool bInOwnerFinished)
{
	if (IsServerTask())
	{
		/* Make one last check to ensure our client and server are synced. Does nothing if they're out of sync, so this
		 * is harmless. */
		if (EquipmentComponent)
		{
			EquipmentComponent->Client_EquipPredictionFailed(bEquipTemporarily);
		}

		// Clear callbacks.
		if (AbilitySystemComponent.Get())
		{
			AbilitySystemComponent->AbilityFailedCallbacks.RemoveAll(this);
		}
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_EquipSet::OnAbilityFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& Reason)
{
	// If this task's outer ability fails, we have to revert our predicted equip.
	if (FailedAbility == Ability && IsServerTask() && EquipmentComponent)
	{
		EquipmentComponent->Client_EquipPredictionFailed(bEquipTemporarily);
	}
}

bool UAbilityTask_EquipSet::IsServerTask() const
{
	return Ability && Ability->GetCurrentActorInfo()->IsNetAuthority();
}
