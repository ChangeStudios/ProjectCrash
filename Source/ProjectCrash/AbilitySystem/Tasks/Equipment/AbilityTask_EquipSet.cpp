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
			if (!Ability->GetCurrentActorInfo()->IsNetAuthority())
			{
				EquipmentComponent->bPredictedUnequip = true;

				// The equipped set, which will be equipped when the temporary set is unequipped, is now the predicted set.
				EquipmentComponent->PredictedEquipmentSetHandle = FEquipmentSetHandle(EquipmentComponent->EquippedSetHandle);
			}
		}

	}
}

void UAbilityTask_EquipSet::Activate()
{
	Super::Activate();

	const bool bAuthority = Ability->GetCurrentActorInfo()->IsNetAuthority();
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
	if (Ability && Ability->GetCurrentActorInfo()->IsNetAuthority())
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
	if (FailedAbility == Ability && Ability->GetCurrentActorInfo()->IsNetAuthority() && EquipmentComponent)
	{
		EquipmentComponent->Client_EquipPredictionFailed(bEquipTemporarily);
	}
}
