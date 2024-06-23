// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilityTask_WaitReusableTargetData.generated.h"

class AGameplayAbilityTargetActor;

/**
 * Waits to receive target data from a specified target data actor. When called on a client, waits for target data from
 * the target data actor, then sends that data to the server. When called on the server, waits for target data from the
 * target data actor and immediately uses that data.
 *
 * This is an optimized version of the UAbilityTask_WaitTargetData task. UAbilityTask_WaitTargetData creates and
 * destroys the target actor each time the task is called, which is not performant. This task can re-configure and
 * re-use an actor each ability activation. This task also supports multiple targets.
 *
 * The spawning and re-configuring of the target actor must be handled by the ability, as this task is agnostic to the
 * actual type of target actor.
 *
 * The UAbilityTask_WaitTargetData class isn't well-suited for subclassing; we do a full rewrite here.
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
	 * The target actor should be spawned and configured before being passed into this task. For instanced-per-actor
	 * abilities, the target actor should only be spawned for the first activation, and re-configured and re-used for
	 * subsequent activations for optimal performance.
	 *
	 * @param TaskInstanceName							Overrides the name of this task for later querying.
	 * @param ConfirmationType							How this targeting information will be confirmed. "Instant"
	 *													targeting will be confirmed automatically. Otherwise, target
	 *													data must be confirmed manually.
	 * @param InTargetActor								The target actor being used by this ability. This should
	 *													be created and configured by this ability before the task
	 *													begins.
	 * @param bCreateKeyIfNotValidForMorePredicting		Whether to create a new scoped prediction key if the current key
	 *													is no longer valid for predicting. Always creates a new scoped
	 *													prediction key by default. Set this to "false" to use an
	 *													existing key, like an ability's activation key for a batched
	 *													ability.
	 */
	UFUNCTION(BlueprintCallable,  Category = "Ability|Tasks", Meta = (DisplayName = "WaitTargetDataWithReusableActor",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"))
	static UAbilityTask_WaitReusableTargetData* CreateWaitTargetDataWithReusableActorProxy
	(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor,
		bool bCreateKeyIfNotValidForMorePredicting = true
	);



	// Task initialization.

protected:

	/** Initializes target actor, starts targeting, and begins waiting to receive target data. */
	virtual void Activate() override;

	/**
	 * Registers callbacks for receiving target data locally. If a client is generating target data, this starts
	 * listening for when to send the data to the server. If the server is generating the data directly, this starts
	 * listening for when the data is ready to use.
	 *
	 * This can be overridden to perform additional actor-specific configurations on the target actor. 
	 */
	virtual void ConfigureTargetActor();

	/**
	 * Registers server-side callbacks for receiving replicated target data from the client, if data is being
	 * generated and sent by the client.
	 *
	 * If the server is generating the data directly, this does nothing. The local callbacks bound in
	 * ConfigureTargetActor are used instead.
	 */
	void RegisterServerTargetDataCallbacks();

	/**
	 * Sets up the target actor's confirmation logic. Should be called AFTER targeting starts.
	 *
	 * If the target data should be instantly confirmed (i.e. confirmation type is Instant), it is confirmed here.
	 * Otherwise, this registers to callbacks for when the target data is manually confirmed or cancelled.
	 *
	 * This can be overridden to set up more complex confirmation logic.
	 */
	virtual void SetUpTargetingConfirmation();

public:

	/** Clean-up when this task is ended. */
	virtual void OnDestroy(bool bInOwnerFinished) override;



	// Task logic.

public:

	/** Confirms this task's pending target data, if the data is not confirmed instantly. I.e. confirmation type is not
	 * Instant. */
	virtual void ExternalConfirm(bool bEndTask) override;

	/** Cancels this task. Broadcasts the current target data via the Cancelled delegate, even if the target data
	 * wasn't confirmed. */
	virtual void ExternalCancel() override;



	// Targeting callbacks. Only executed on the machine generating the data.

protected:

	/** Called on clients when the target data is ready to be sent to the server. If target data is generated by the
	 * server directly, this is called on the server when it generates the data. */
	UFUNCTION()
	virtual void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data);

	/** Called on clients when targeting is locally cancelled. If target data is generated by the server directly,
	 * called on the server if it cancels targeting. */
	UFUNCTION()
	virtual void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);



	/* Server-side targeting callbacks. Executed on the server if the data is being generated by a client and sent to
	 * the server. */

protected:

	/** Called when the server receives target data from a client. Is not called if the server generates target data
	 * directly. */
	UFUNCTION()
	virtual void OnTargetDataReplicated(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Activation);

	/** Called when the server receives that targeting was cancelled by a client. Is not called if the server generates
	 * target data directly. */
	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelled();



	// Task output pins.

public:

	/** Fired on the client when valid target data is sent to the server, so we can predict any subsequent logic. Fired
	 * on the server when valid target data is received from a client, or is generated directly on the server. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnTargetDataReady")
	FWaitTargetDataDelegate TargetDataReadyDelegate;

	/** Fired on the client when targeting is cancelled locally. Fired on the server upon being informed by a client
	 * that targeting was cancelled, or when targeting is cancelled locally. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnCancelled")
	FWaitTargetDataDelegate TargetingCancelledDelegate;



	// Task parameters.

protected:

	/** The target actor being used for this task. For instanced-per-actor abilities, should be spawned once, the first
	 * time this task is called, and reconfigured for each subsequent use. */
	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> TargetActor;

	/** How this ability will be confirmed. "Instant" targeting will be automatically confirmed when target data is
	 * generated. Otherwise, the target data must be confirmed manually. */
	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	/** If true, this ability task will create a new scoped prediction key if the current key is no longer valid.
	 * Setting this to false enables this task's ability to use an existing key, such as an activation key for the
	 * ability, if it was batched. */
	bool bCreateKeyIfNotValidForMorePredicting;



	// Utils.

protected:

	/**
	 * Whether target data should be sent to the server by this task.
	 *
	 * Target data should only be sent to the server if (1) this task is executing on the client and (2) this task is
	 * not using a target actor that produces data on the server directly.
	 */
	bool ShouldReplicateDataToServer();
};
