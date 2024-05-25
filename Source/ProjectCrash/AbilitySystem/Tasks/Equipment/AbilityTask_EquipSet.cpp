// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Tasks/Equipment/AbilityTask_EquipSet.h"

#include "Equipment/EquipmentComponent.h"
#include "GameFramework/CrashLogging.h"


UAbilityTask_EquipSet* UAbilityTask_EquipSet::EquipSet(UGameplayAbility* OwningAbility, FName TaskInstanceName, UEquipmentSetDefinition* SetToEquip)
{
	// Create a new task.
	UAbilityTask_EquipSet* MyTask = NewAbilityTask<UAbilityTask_EquipSet>(OwningAbility, TaskInstanceName);

	// Cache this task's parameters.
	MyTask->SetToEquip = SetToEquip;

	return MyTask;
}

void UAbilityTask_EquipSet::Activate()
{
	Super::Activate();

	/* Cache the current set
	 *
	 * Equip the set locally
	 *
	 * wait for the server to equip the set
	 *
	 * Unequip the set locally
	 */

	UEquipmentComponent* EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(Ability->GetAvatarActorFromActorInfo());

	if (!EquipmentComponent)
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to activate AbilityTask_EquipSet, but the ability's avatar [%s] does not have an equipment component!"), *GetNameSafe(Ability->GetAvatarActorFromActorInfo()));
		EndTask();
	}

	PreviousSet = EquipmentComponent->GetEquippedSet();
}
