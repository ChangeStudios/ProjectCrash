// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashAbilitySystemGlobals.h"

#include "CrashGameplayAbilityTypes.h"
#include "Components/CrashAbilitySystemComponent.h"
#include "GameplayEffects/CrashGameplayEffectContext.h"

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

	// Return nullptr, even if an ASC was found, if it's not the correct type.
	return CrashASC ? CrashASC : nullptr;
}

void UCrashAbilitySystemGlobals::InitGameplayCueParameters(FGameplayCueParameters& CueParameters, const FGameplayEffectContextHandle& EffectContext)
{
	if (EffectContext.IsValid())
	{
		CueParameters.EffectContext = EffectContext;

		if (const FHitResult* HitResult = EffectContext.GetHitResult())
		{
			CueParameters.Location = (HitResult->bBlockingHit ? HitResult->ImpactPoint : HitResult->TraceEnd);
			CueParameters.Normal = HitResult->ImpactNormal;
			CueParameters.PhysicalMaterial = HitResult->PhysMaterial;
			CueParameters.TargetAttachComponent = (HitResult->Component.IsValid() ? HitResult->Component.Get() : HitResult->GetActor()->GetRootComponent());
		}

		// NOTE: We might have to initialize the cue's instigator, effect causer, etc. too.
	}
}
