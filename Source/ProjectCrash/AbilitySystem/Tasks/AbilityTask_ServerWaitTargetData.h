// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilityTask_WaitReusableTargetData.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_ServerWaitTargetData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_ServerWaitTargetData : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataDelegate ValidDataReceivedDelegate;

	/**
	 * Instructs the server to wait to receive target data from the client.
	 *
	 * @param OwningAbility			The ability instance executing this task.
	 * @param TaskInstanceName		An optional name assigned to this ability task instance that can later be used to
	 *								reference it.
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UAbilityTask_ServerWaitTargetData* ServerWaitForClientTargetData(UGameplayAbility* OwningAbility, FName TaskInstanceName);

	virtual void Activate() override;

	UFUNCTION()
	void OnTargetDataSet(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

protected:

	virtual void OnDestroy(bool bInOwnerFinished) override;
};
