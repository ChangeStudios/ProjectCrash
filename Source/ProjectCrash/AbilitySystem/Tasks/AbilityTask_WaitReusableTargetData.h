// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilityTask_WaitReusableTargetData.generated.h"

class AGameplayAbilityTargetActor;

/**
 * Waits to receive target data from a specified target data actor. When called on a client, waits for target data from
 * the target data actor, then sends that data to the server. When called on the server, waits for target data from the
 * target data actor and then uses that data.
 *
 * This is an optimized version of the UAbilityTask_WaitTargetData task. UAbilityTask_WaitTargetData creates and
 * destroys the target actor each time this task is called, which is not performant. This task only creates one target
 * actor for each ability and re-uses that actor, re-configuring it for each ability activation. This task also
 * supports multiple targets, enabling support for abilities that have multiple targets.
 *
 * This task is intended for use in instanced-per-actor abilities. It will work in instanced-per-activation abilities,
 * but the target actor won't be cached between activations, resulting in similar behavior to
 * UAbilityTask_WaitTargetData.
 *
 * The UAbilityTask_WaitTargetData class isn't well-suited for subclassing, so we do a full rewrite here.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_WaitReusableTargetData : public UAbilityTask
{
	GENERATED_BODY()

	// Task construction.

public:

	/**
	 * Waits to receive target data from the given target actor. When called on a client, waits for target data from
	 * the target data actor, then sends that data to the server. When called on the server, waits for target data from
	 * the target data actor and immediately uses that data.
	 *
	 * The target actor should be created and configured before it's passed into this task.
	 *
	 * @param OwningAbility								The ability instance executing this task.
	 * @param TaskInstanceName							An optional name assigned to this ability task instance that can
	 *													later be used to reference it.
	 * @param ConfirmationType							How this targeting information will be confirmed. "Instant"
	 *													targeting will be confirmed automatically. Otherwise, target
	 *													data must be confirmed manually.
	 * @param InTargetActor								The target actor being used by this ability. This should
	 *													be created, configured, and cached by this ability before this
	 *													task begins.
	 * @param bCreateKeyIfNotValidForMorePredicting		Whether to create a new scoped prediction key if the current key
	 *													is no longer valid for predicting. Always creates a new scoped
	 *													prediction key by default. Set this to "false" to use an
	 *													existing key, like an ability's activation key for a batched
	 *													ability.
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAbilityTask_WaitReusableTargetData* WaitTargetDataWithReusableActor
	(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor,
		bool bCreateKeyIfNotValidForMorePredicting = true
	);



	// Task initialization.

protected:

	/** Begins waiting to receive target data. */
	virtual void Activate() override;

	/** Called when this task is ended. */
	virtual void OnDestroy(bool bInOwnerFinished) override;

	/** Registers server-side callbacks for receiving replicated target data from the client, if data is being
	 * generated and sent by the client (as opposed to the server generating the data directly, in which case this does
	 * nothing, as the local callbacks are used instead). */
	UFUNCTION()
	void RegisterTargetDataCallbacks();

	/** Registers callbacks for receiving target data locally (i.e. when the client sends data to the server or when
	 * the server generates the data if it does so directly). */
	virtual void ConfigureTargetActor();

	/**
	 * Sets up the target actor's confirmation logic.
	 *
	 * If the target data should be instantly confirmed (i.e. its confirmation type is Instant), it is confirmed here.
	 * Otherwise, registers to callbacks for when the target data is confirmed or cancelled.
	 */
	virtual void FinalizeTargetActorInitialization();



	// Task logic.

public:

	/** Confirms this task's pending target data, if the data is not confirmed instantly (i.e. confirmation type is not
	 * Instant). */
	virtual void ExternalConfirm(bool bEndTask) override;

	/** Cancels this task. Broadcasts the current target data via the Cancelled delegate, even if the target data
	 * wasn't confirmed. */
	virtual void ExternalCancel() override;

	/** Returns whether target data should be sent to the server by this task. Target data should only be sent to the
	 * server if (1) this task is executing on the client and (2) this task is not using a GameplayTargetActor that
	 * produces data on the server directly. */
	bool ShouldReplicateDataToServer();



	// Local callbacks. Executed on the machine generating the data.

protected:

	/** Called on clients when the target data is ready to be sent to the server. If target data is generated by the
	 * server directly, this is called on the server when it generates the data. */
	UFUNCTION()
	virtual void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);

	/** Called on clients when targeting is locally cancelled. If target data is generated by the server directly,
	 * called on the server if it cancels targeting. */
	UFUNCTION()
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);



	// Server-side callbacks. Executed on the server if the data is being generated and sent by a client.

protected:

	/** Called when the server receives target data from a client. Is not called if the server generates target data
	 * directly. */
	UFUNCTION()
	virtual void OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Activation);

	/** Called when the server receives that targeting was cancelled by a client. Is not called if the server generates
	 * target data directly. */
	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelled();



	// Delegates.

public:

	/** Broadcast on the client when valid target data is sent to the server. Broadcast on the server when valid target
	 * data is received by a client or is generated directly on the server. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Valid Data")
	FWaitTargetDataDelegate ValidDataSentDelegate;

	/** Broadcast on the client when targeting is cancelled locally. Broadcast on the server when receiving that
	 * targeting was cancelled by a client, or when targeting is cancelled locally. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Cancelled")
	FWaitTargetDataDelegate CancelledDataDelegate;



	// Task parameters.

protected:

	/** The target actor being used for this task. This should be spawned once, the first time this task is called, and
	 * reconfigured subsequent execution. */
	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> TargetActor;

	/** How this ability will be confirmed. "Instant" targeting will be automatically confirmed when target data is
	 * sent. Otherwise, the target data must be confirmed manually. */
	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	/** If true, this ability task will create a new scoped prediction key if the current key is no longer valid.
	 * Setting this to false enables this task's ability to use an existing key, such as an activation key for the
	 * ability if it was batched. */
	bool bCreateKeyIfNotValidForMorePredicting;
};
