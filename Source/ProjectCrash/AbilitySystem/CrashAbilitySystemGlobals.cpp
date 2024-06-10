// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashAbilitySystemGlobals.h"

#include "CrashGameplayAbilityTypes.h"
#include "Components/CrashAbilitySystemComponent.h"
#include "Effects/CrashGameplayEffectContext.h"

FGameplayEffectContext* UCrashAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FCrashGameplayEffectContext();
}

FGameplayAbilityActorInfo* UCrashAbilitySystemGlobals::AllocAbilityActorInfo() const
{
	return new FCrashGameplayAbilityActorInfo();
}

UCrashAbilitySystemComponent* UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent)
{
	// Use the default method to try and find an ASC and cast it to a Crash ASC.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActor(Actor, LookForComponent);
	UCrashAbilitySystemComponent* CrashASC = ASC ? Cast<UCrashAbilitySystemComponent>(ASC) : nullptr;

	// Return nullptr even if an ASC was found if it's not the correct type.
	return CrashASC ? CrashASC : nullptr;
}
