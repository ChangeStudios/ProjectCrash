// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/InstigatorEffectsGameplayEffectComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"

void UInstigatorEffectsGameplayEffectComponent::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	if (!ensureMsgf(ActiveGEContainer.Owner, TEXT("OnGameplayEffectApplied is passed an ActiveGEContainer which lives within an ASC but that ASC was somehow null")))
	{
		return;
	}

	// Collect GE data.
	const float GELevel = GESpec.GetLevel();
	const FGameplayEffectContextHandle& GEContextHandle = GESpec.GetEffectContext();
	AActor* Instigator = GEContextHandle.GetInstigator();

	if (!IsValid(Instigator))
	{
		return;
	}

	// Create specs for each effect that should be applied.
	TArray<FGameplayEffectSpecHandle> EffectSpecsToApply;
	for (const FConditionalGameplayEffect& ConditionalEffect : GameplayEffectsToApply)
	{
		const UGameplayEffect* GameplayEffectDef = ConditionalEffect.EffectClass.GetDefaultObject();
		if (!GameplayEffectDef)
		{
			continue;
		}

		/* Use the tags of the target of this GE as the source, instead of the source of this GE (we're switching the
		 * target and source). */
		if (ConditionalEffect.CanApply(GESpec.CapturedTargetTags.GetActorTags(), GELevel))
		{
			FGameplayEffectSpecHandle SpecHandle;
			SpecHandle = FGameplayEffectSpecHandle(new FGameplayEffectSpec());

			// NOTE: We may need to manually switch the target data and source data.
			SpecHandle.Data->InitializeFromLinkedSpec(GameplayEffectDef, GESpec);

			if (ensure(SpecHandle.IsValid()))
			{
				EffectSpecsToApply.Add(SpecHandle);
			}
		}
	}

	// Apply the desired gameplay effects to the SOURCE of this effect, NOT the target.
	UAbilitySystemComponent& TargetASC = *ActiveGEContainer.Owner;
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Instigator);
	for (const FGameplayEffectSpecHandle& EffectSpec : EffectSpecsToApply)
	{
		if (EffectSpec.IsValid())
		{
			TargetASC.ApplyGameplayEffectSpecToTarget(*EffectSpec.Data.Get(), SourceASC, PredictionKey);
		}
	}
}