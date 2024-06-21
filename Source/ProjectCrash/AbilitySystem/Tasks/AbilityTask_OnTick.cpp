// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Tasks/AbilityTask_OnTick.h"

UAbilityTask_OnTick::UAbilityTask_OnTick(const FObjectInitializer& ObjectInitializer)
{
	bTickingTask = true;
	TaskDeltaTime = 0.0f;
}

UAbilityTask_OnTick* UAbilityTask_OnTick::OnAbilityTick(UGameplayAbility* OwningAbility, FName TaskInstanceName)
{
	// Instantiate the new ability task.
	UAbilityTask_OnTick* MyObj = NewAbilityTask<UAbilityTask_OnTick>(OwningAbility);
	return MyObj;
}

void UAbilityTask_OnTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	// Broadcast the OnTick delegate each tick.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}