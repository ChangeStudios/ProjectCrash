// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectComponent.h"
#include "KnockbackGameplayEffectComponent.generated.h"

/**
 * Applies knockback to the avatar actor of the ASC to which this effect is applied. Knockback direction is determined
 * by the "Normal" value of the effect context's hit result.
 */
UCLASS(CollapseCategories, DisplayName = "Apply Knockback", Meta = (ToolTip = "Applies knockback to the avatar actor of the ASC to which this effect is applied. Knockback direction is determined by the \"Normal\" value of the effect context's hit result."))
class PROJECTCRASH_API UKnockbackGameplayEffectComponent : public UGameplayEffectComponent
{
	GENERATED_BODY()

public:

	/** Returns the amount of force that this effect should apply, in cm/s. */
	FORCEINLINE FVector GetKnockbackForce(const FVector Direction) const { return (Direction * Magnitude * 100.0f); } // Convert to cm/s.

protected:

	/** Multiplied by the knockback direction (the effect context's hit result's "Normal" parameter) to get the final
	 * vector used to launch the target. */
	UPROPERTY(EditDefaultsOnly, Category = "Apply Knockback", Meta = (Units = "MetersPerSecond"))
	float Magnitude;
};
