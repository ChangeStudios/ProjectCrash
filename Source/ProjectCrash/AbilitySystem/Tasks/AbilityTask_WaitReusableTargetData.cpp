// Copyright Samuel Reitich 2024.


#include "AbilityTask_WaitReusableTargetData.h"

#include "Abilities/GameplayAbilityTargetActor.h"

UAbilityTask_WaitReusableTargetData* UAbilityTask_WaitReusableTargetData::WaitTargetDataWithReusableActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor, bool bAllowMultipleTargets, bool bCreateKeyIfNotValidForMorePredicting)
{
	// Don't perform targeting without a valid target actor.
	if (!IsValid(InTargetActor))
	{
		return nullptr;
	}

	// Create this a new task.
	UAbilityTask_WaitReusableTargetData* MyTask = NewAbilityTask<UAbilityTask_WaitReusableTargetData>(OwningAbility, TaskInstanceName);

	// Cache this task's parameters.
	MyTask->TargetActor = InTargetActor;
	MyTask->ConfirmationType = ConfirmationType;
	MyTask->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;

	return MyTask;
}

void UAbilityTask_WaitReusableTargetData::ExternalConfirm(bool bEndTask)
{
	// Confirm this task's targeting data, if it should send targeting data.
	if (TargetActor)
	{
		if (TargetActor->ShouldProduceTargetData())
		{
			TargetActor->ConfirmTargetingAndContinue();
		}
	}

	Super::ExternalConfirm(bEndTask);
}

void UAbilityTask_WaitReusableTargetData::ExternalCancel()
{
	// Broadcast the cancellation of this task's targeting.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		CancelledDataDelegate.Broadcast(FGameplayAbilityTargetData());
	}
	
	Super::ExternalCancel();
}

void UAbilityTask_WaitReusableTargetData::Activate()
{
	if (!IsValid(this))
	{
		return;
	}

	// Initialize the target data actor to generate and send the targeting data.
	if (Ability && TargetActor)
	{
		ConfigureTargetActor();
		RegisterTargetDataCallbacks();
		FinalizeTargetActorInitialization();
	}
	/* End this task immediately if it doesn't have a targeting actor. This should never happen, since the task should
	 * never begin without a target actor. */
	else
	{
		EndTask();
	}
}

void UAbilityTask_WaitReusableTargetData::OnDestroy(bool bInOwnerFinished)
{
	if (TargetActor)
	{
		TargetActor->SetActorTickEnabled(false);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitReusableTargetData::RegisterTargetDataCallbacks()
{
}

bool UAbilityTask_WaitReusableTargetData::ShouldReplicateDataToServer()
{
	return true;
}

void UAbilityTask_WaitReusableTargetData::ConfigureTargetActor()
{
}

void UAbilityTask_WaitReusableTargetData::FinalizeTargetActorInitialization()
{
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data)
{
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data,
	FGameplayTag Activation)
{
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReplicatedCancelled()
{
}
