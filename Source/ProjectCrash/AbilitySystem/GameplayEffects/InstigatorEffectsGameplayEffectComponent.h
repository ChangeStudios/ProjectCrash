// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"
#include "InstigatorEffectsGameplayEffectComponent.generated.h"

/**
 * Add additional gameplay effects to the instigator of this effect (not the target). Effects will only be applied if
 * there is a valid instigator in the effect's context.
 */
UCLASS(CollapseCategories, DisplayName="Apply Effects to Instigator")
class PROJECTCRASH_API UInstigatorEffectsGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()

public:

	/** Applies this component's defined gameplay effects to the source/instigator of the parent effect. */
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;

public:

	/** Gameplay effects that will be applied to the instigator of this gameplay effect. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FConditionalGameplayEffect> GameplayEffectsToApply;
};
