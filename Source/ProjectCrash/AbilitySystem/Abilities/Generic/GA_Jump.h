// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GA_Jump.generated.h"

/**
 * The default "jump" ability. Avatar must be of the Character class, as this ability uses the built-in character jump
 * method.
 *
 * This ability's properties are determined by the avatar's Character properties and movement component.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UGA_Jump : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

protected:

	/** Ensures character can jump. Checks conditions like available space, if the character is airborne, if the
	 * character has remaining jumps, etc. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	/** Attempts to trigger the built-in character jump method. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Forces the StopJumping method in case it wasn't handled automatically. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
