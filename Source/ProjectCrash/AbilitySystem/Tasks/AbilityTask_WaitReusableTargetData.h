// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitReusableTargetData.generated.h"

/**
 * TODO: Implement this to use a reusable target data actor, which will solve the performance problems of the other WaitTargetData tasks.
 *
 * TODO: Implement another task here that supports multiple async target data actor returns and can perform logic with each when they are received.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_WaitReusableTargetData : public UAbilityTask
{
	GENERATED_BODY()
	
};
