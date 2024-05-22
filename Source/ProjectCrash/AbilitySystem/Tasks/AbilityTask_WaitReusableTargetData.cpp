// Copyright Samuel Reitich 2024.


#include "AbilityTask_WaitReusableTargetData.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameFramework/CrashLogging.h"

UAbilityTask_WaitReusableTargetData* UAbilityTask_WaitReusableTargetData::WaitTargetDataWithReusableActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor, bool bCreateKeyIfNotValidForMorePredicting)
{
	// Don't perform targeting without a valid target actor.
	if (!IsValid(InTargetActor))
	{
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
		CancelledDataDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
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
	/* End this task immediately if it doesn't have a target actor. This should never happen, since the task should
	 * never begin without a target actor. */
	else
	{
		EndTask();
	}
}

void UAbilityTask_WaitReusableTargetData::OnDestroy(bool bInOwnerFinished)
{
	// Disable ticking on the target actor when this task ends.
	if (TargetActor)
	{
		TargetActor->SetActorTickEnabled(false);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitReusableTargetData::RegisterTargetDataCallbacks()
{
	if (!ensure(IsValid(this)))
	{
		return;
	}

	check(Ability);

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

bool UAbilityTask_WaitReusableTargetData::ShouldReplicateDataToServer()
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	/* Only send target data if (1) this is the client and (2) the current target actor does not produce data on the
	 * server directly. */
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
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

void UAbilityTask_WaitReusableTargetData::FinalizeTargetActorInitialization()
{
	check(TargetActor);
	check(Ability);

	TargetActor->StartTargeting(Ability);

	// Set up confirmation logic.
	if (TargetActor->ShouldProduceTargetData())
	{
		switch (ConfirmationType)
		{
			/* Automatically confirm the target data as soon as the target actor is initialized if it should be
			 * confirmed as soon as it's generated. */
			case EGameplayTargetingConfirmation::Instant:
			{
				TargetActor->ConfirmTargeting();
				break;
			}
			/* Bind to external confirmation and cancellation delegates if the target data should be manually confirmed
			 * by the player at some later point. */
			case EGameplayTargetingConfirmation::UserConfirmed:
			{
				TargetActor->BindToConfirmCancelInputs();
				break;
			}
			// Any other type of confirmation should implement its own logic for confirming target data.
			default:
			{
			}
		}
	}
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
		 * server will have confirmed the target data itself and won't need this event. */
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	/* Locally broadcast that the target data was generated. Executes on the client when the target data is sent to the
	 * server. Executes on the server if the target data was generated directly by the server. */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidDataSentDelegate.Broadcast(Data);
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
		/* If target data is generated directly on the server, send a generic event to the server informing it of the
		 * cancellation. */
		else
		{
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	// Broadcast that targeting was cancelled.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		CancelledDataDelegate.Broadcast(Data);
	}

	// Always end this task when targeting is cancelled.
	EndTask();
}

void UAbilityTask_WaitReusableTargetData::OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Activation)
{ 
	FGameplayAbilityTargetDataHandle MutableData = Data;

	// Clean the target data.
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
			CancelledDataDelegate.Broadcast(MutableData);
		}
	}
	else
	{
		// Broadcast that the server received the data if it was generated and sent by the client.
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidDataSentDelegate.Broadcast(MutableData);
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
	// Broadcast the targeting cancellation.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		CancelledDataDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}

	// End the task on the server when told that the client cancelled targeting.
	EndTask();
}
