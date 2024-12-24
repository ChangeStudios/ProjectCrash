// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponent.h"
#include "GameplayEffectUIData.h"
#include "GameplayTagContainer.h"
#include "UIExtensionSystem.h"
#include "WidgetGameplayEffectComponent.generated.h"

/**
 * Pushes a widget representing the gameplay effect while the effect is active. Can represent buffs or debuffs. A timer
 * will be displayed for duration-based effects.
 *
 * TODO: Finish.
 */
UCLASS(CollapseCategories, DisplayName = "Add Effect Widget")
class PROJECTCRASH_API UWidgetGameplayEffectComponent : public UGameplayEffectUIData
{
	GENERATED_BODY()

public:

	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;

	UFUNCTION()
	void OnGameplayEffectRemoved(const FActiveGameplayEffect& ActiveGE) const;

public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> Widget;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WidgetSlot;

#if WITH_EDITOR
	/** Prevents this component from being added to instant effects. Though, in the future, we may want to add an
	 * indicator or pop-up when an instant effect was applied. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
