// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilityTask_WaitReusableTargetData.generated.h"

class AGameplayAbilityTargetActor;

/**
 * Waits to receive target data from a specified target
 * data actor. This should be called on clients to generate target data before sending that data to the server with
 * UAbilityTask_ServerWaitTargetData.
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
	 * Waits to receive target data from the given target actor. This should be called on clients to generate
	 * target data before sending that data to the server with UAbilityTask_ServerWaitTargetData.An optimized version of the WaitTargetData task. This task will use the given target actor to generate and
	 * return target data without destroying the target actor.
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



	// Task logic.

public:

	/** Confirms this task's pending target data, if the data is not confirmed instantly (i.e. confirmation type is not
	 * Instant). */
	virtual void ExternalConfirm(bool bEndTask) override;

	/** Cancels this task. Broadcasts the current target data via the Cancelled delegate, even if the target data
	 * wasn't confirmed. */
	virtual void ExternalCancel() override;

protected:

	/** Begins waiting to receive target data. */
	virtual void Activate() override;

	/** Called when this task is ended. */
	virtual void OnDestroy(bool bInOwnerFinished) override;

	/** Registers this task's callbacks to the ability's ASC's delegates to broadcast when the targeting data is
	 * ready. */
	UFUNCTION()
	void RegisterTargetDataCallbacks();

	/** Returns whether target data should be sent to the server by this task. Target data should only be sent to the
	 * server if (1) this task is executing on the client and (2) this task is not using a GameplayTargetActor that
	 * produces data on the server directly. */
	bool ShouldReplicateDataToServer();

	/** Configures this task's target actor for the next execution of this task. The target actor should reconfigure
	 * itself separately to update actor-specific information, like the target actor's shape. */
	virtual void ConfigureTargetActor();

	/** Sets up the target actor's confirmation logic.
	 *
	 * If the target data should be instantly confirmed (i.e. its confirmation type is Instant), it is confirmed here.
	 * Otherwise, registers to callbacks for when the target data is confirmed or cancelled. */
	virtual void FinalizeTargetActorInitialization();



	// Client-side callbacks.

protected:

	/** Called when the target data is ready to be sent to the server. */
	UFUNCTION()
	virtual void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);

	/** Called when a client cancels targeting. */
	UFUNCTION()
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);



	// Server-side callbacks.

protected:

	/** Called when the server receives target data from a client. */
	UFUNCTION()
	virtual void OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Activation);

	/** Called when the server receives that targeting was cancelled by a client. */
	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelled();



	// Delegates.

protected:

	/** Broadcast when valid target data is sent to server. */
	UPROPERTY(BlueprintAssignable, DisplayName = "Valid Data")
	FWaitTargetDataDelegate ValidDataSentDelegate;

	/** Broadcast when targeting is cancelled. */
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
