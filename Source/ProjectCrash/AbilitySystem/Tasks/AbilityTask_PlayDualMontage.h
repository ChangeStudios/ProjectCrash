// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PlayDualMontage.generated.h"

/**
 * Plays an animation montage on the first-person mesh for the local client and plays a replicated montage on the
 * third-person mesh for all clients. Must be called with authority to replicate the third-person montage.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_PlayDualMontage : public UAbilityTask
{
	GENERATED_BODY()
	
};
