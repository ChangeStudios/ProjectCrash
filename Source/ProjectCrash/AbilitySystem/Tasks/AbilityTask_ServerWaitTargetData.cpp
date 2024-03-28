// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Tasks/AbilityTask_ServerWaitTargetData.h"

#include "AbilitySystemComponent.h"

UAbilityTask_ServerWaitTargetData* UAbilityTask_ServerWaitTargetData::ServerWaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName)
{
	// Create a new task.
	UAbilityTask_ServerWaitTargetData* MyTask = NewAbilityTask<UAbilityTask_ServerWaitTargetData>(OwningAbility, TaskInstanceName);
	return MyTask;
}

void UAbilityTask_ServerWaitTargetData::Activate()
{
	// Only run this task on the server.
	if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		return;
	}

	/* Bind a callback to when target data gets set. This will trigger if the client sends target data or if the server
	 * generates its own target data. */
	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
	AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAbilityTask_ServerWaitTargetData::OnTargetDataSet);
}

void UAbilityTask_ServerWaitTargetData::OnTargetDataSet(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;

	// Clean the target data.
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidDataReceivedDelegate.Broadcast(MutableData);
	}

	EndTask();
}

void UAbilityTask_ServerWaitTargetData::OnDestroy(bool bInOwnerFinished)
{
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
	}

	Super::OnDestroy(bInOwnerFinished);
}
