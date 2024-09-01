// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/Generic/CrashGameplayAbility_Death.h"

#include "CrashGameplayTags.h"

UCrashGameplayAbility_Death::UCrashGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData DefaultTrigger;
	DefaultTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	DefaultTrigger.TriggerTag = CrashGameplayTags::TAG_GameplayEvent_Ability_Death;
	AbilityTriggers.Add(DefaultTrigger);
}

void UCrashGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UCrashGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

