// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashGameplayAbility_Death.generated.h"

/**
 * The base class for "Death" abilities. This is triggered for the ASC of dying actors with the
 * "GameplayEvent.Ability.Death" event. This is activated automatically by the HealthAttributeSet and HealthComponent.
 *
 * This should be subclassed to handle client-side death logic, such as changing camera modes or playing VFX. Default
 * implementation cancels active abilities.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashGameplayAbility_Death : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UCrashGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
