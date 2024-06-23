// Copyright Samuel Reitich. All rights reserved.


#include "AbilityTask_WaitReusableTargetData.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameFramework/CrashLogging.h"

UAbilityTask_WaitReusableTargetData* UAbilityTask_WaitReusableTargetData::CreateWaitTargetDataWithReusableActorProxy(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor, bool bCreateKeyIfNotValidForMorePredicting)
{
	// Don't perform targeting without a valid target actor.
	if (!IsValid(InTargetActor))
	{
		ABILITY_LOG(Error, TEXT("UAbilityTask_WaitReusableTargetData called in ability [%s] failed to activate. Null target actor given."), *GetNameSafe(OwningAbility));
		return nullptr;
	}

	// Create a new task.
	UAbilityTask_WaitReusableTargetData* MyTask = NewAbilityTask<UAbilityTask_WaitReusableTargetData>(OwningAbility, TaskInstanceName);

	// Cache this task's parameters.
	MyTask->TargetActor = InTargetActor;
	MyTask->ConfirmationType = ConfirmationType;
	MyTask->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;

	return MyTask;
}

void UAbilityTask_WaitReusableTargetData::ExternalConfirm(bool bEndTask)
{
	// Confirm this task's targeting data, if it should send target data.
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
		TargetingCancelledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}

	Super::ExternalCancel();
}

void UAbilityTask_WaitReusableTargetData::Activate()
{
	if (!IsValid(this))
	{
		return;
	}

	check(Ability);
	check(TargetActor);

	// Initialize the target data actor to generate and send the targeting data.
	ConfigureTargetActor();
	RegisterServerTargetDataCallbacks();

	/* Start targeting BEFORE setting up targeting confirmation. Instant targeting won't have anything to instantly
	 * confirm otherwise. */
	TargetActor->StartTargeting(Ability);
	SetUpTargetingConfirmation();
}

void UAbilityTask_WaitReusableTargetData::RegisterServerTargetDataCallbacks()
{
	const bool bLocallyControlled = Ability->IsLocallyControlled();
	const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;

	// Prepare the server to receive the target data from the client.
	if (!bLocallyControlled)
	{
		// Don't try to receive target data if it should be generated on the server instead.
		if (!bShouldProduceTargetDataOnServer)
		{
			const FGameplayAbilitySpecHandle& SpecHandle = GetAbilitySpecHandle();
			const FPredictionKey& ActivationPredictionKey = GetActivationPredictionKey();

			// Register callbacks to when the client sends or cancels target data.
			AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAbilityTask_WaitReusableTargetData::OnTargetDataReplicated);
			AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAbilityTask_WaitReusableTargetData::OnTargetDataReplicatedCancelled);

			// Check if target data was already sent.
			AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			// Start waiting to receive target data.
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UAbilityTask_WaitReusableTargetData::ConfigureTargetActor()
{
	check(TargetActor);
	check(Ability);

	// Set the target actor's player controller.
	TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	// Register callbacks to when the targeting data is ready to be sent or targeting is cancelled.
	TargetActor->TargetDataReadyDelegate.AddUObject(this, &UAbilityTask_WaitReusableTargetData::OnTargetDataReady);
	TargetActor->CanceledDelegate.AddUObject(this, &UAbilityTask_WaitReusableTargetData::OnTargetDataCancelled);
}

void UAbilityTask_WaitReusableTargetData::SetUpTargetingConfirmation()
{
	check(TargetActor);
	check(Ability);

	// Set up target confirmation logic.
	if (TargetActor->ShouldProduceTargetData())
	{
		switch (ConfirmationType)
		{
			// Instant targeting automatically confirms the target data as soon as the target actor starts targeting.
			case EGameplayTargetingConfirmation::Instant:
			{
				TargetActor->ConfirmTargeting();
				break;
			}
			/* Bind to external confirmation and cancellation delegates if the target data should be manually confirmed
			 * at some later point. */
			case EGameplayTargetingConfirmation::UserConfirmed:
			{
				TargetActor->BindToConfirmCancelInputs();
				break;
			}
			// Any other target confirmation method should implement its own logic for confirming target data.
			default:
			{
			}
		}
	}
}

void UAbilityTask_WaitReusableTargetData::OnDestroy(bool bInOwnerFinished)
{
	// Disable ticking on the target actor when this task ends, since we aren't using it, but also aren't destroying it.
	if (TargetActor)
	{
		TargetActor->SetActorTickEnabled(false);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC || !Ability)
	{
		return;
	}

	// Create a new prediction window for sending the target data.
	const bool bCanGenerateNewKey = ShouldReplicateDataToServer() &&
			(bCreateKeyIfNotValidForMorePredicting && !ASC->ScopedPredictionKey.IsValidForMorePrediction());
	FScopedPredictionWindow ScopedPrediction(ASC, bCanGenerateNewKey);

	// Handle predicting clients' communication with the server.
	if (IsPredictingClient())
	{
		// If the client generated the target data, send it to the server while predicting it locally.
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			// If we wanted to pass a gameplay tag in the payload alongside the target data, we could do so here.
			const FGameplayTag ApplicationTag;

			// Handle target data replication and local prediction.
			ASC->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, ASC->ScopedPredictionKey);
		}
		/* If target data is generated directly on the server but had to have been confirmed by the client, send a
		 * generic event to the server informing it of the user's confirmation. For any other confirmation method, the
		 * server will have confirmed the target data itself and won't need this event from the client. */
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	/* Trigger the TargetDataReady output pin when target data is ready. Fired on the client if the target data is
	 * generated by the client and sent to the server. Fired on the server if the target data was generated directly by
	 * the server. */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		TargetDataReadyDelegate.Broadcast(Data);
	}

	/* Automatically end this task after sending target data once, unless it is allowed to send target data multiple
	 * times. */
	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	// Create a new prediction window for cancelling the targeting.
	const bool bCanGenerateNewKey = ShouldReplicateDataToServer() &&
			(bCreateKeyIfNotValidForMorePredicting && !ASC->ScopedPredictionKey.IsValidForMorePrediction());
	FScopedPredictionWindow ScopedPrediction(ASC, bCanGenerateNewKey);

	// Handle predicting clients' communication with the server.
	if (IsPredictingClient())
	{
		/* If the target data was being generated by the client, send the cancellation to the server while predicting
		 * it locally. */
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
		/* If target data was being generated directly on the server, send a generic event to the server informing it
		 * of the cancellation. */
		else
		{
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	// Trigger the TargetingCancelled output pin when targeting is cancelled.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		TargetingCancelledDelegate.Broadcast(Data);
	}

	// Always end this task when targeting is cancelled.
	EndTask();
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Activation)
{ 
	FGameplayAbilityTargetDataHandle MutableData = Data;

	// Consume the target data on the server.
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	/**
	 * Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting the replicated
	 * target data and will treat this as a cancel.
	 *	
	 * This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could generate the data on the
	 * server (e.g. perform a trace, a check, or anything else) and use that data. So rather than having the client
	 * generate and send that data directly, the client can essentially send a confirmation to the server, telling it
	 * to generate the data itself.
	 */
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			TargetingCancelledDelegate.Broadcast(MutableData);
		}
	}
	// If the replicated data was accepted, the data was successfully received and processed by the server.
	else
	{
		/* Trigger the TargetDataReady output pin when the server receives the data, if it was generated and sent by the
		 * client. */
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			TargetDataReadyDelegate.Broadcast(MutableData);
		}
	}

	// End this task after sending target data once, unless it is allowed to send target data multiple times.
	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReplicatedCancelled()
{
	// Trigger the TargetingCancelled when the server receives that targeting was cancelled.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		TargetingCancelledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}

	// End the task on the server when told that the client cancelled targeting.
	EndTask();
}

bool UAbilityTask_WaitReusableTargetData::ShouldReplicateDataToServer()
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	/* Only send target data to the server if (1) this is the client and (2) the current target actor does not produce
	 * data on the server directly. */
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}