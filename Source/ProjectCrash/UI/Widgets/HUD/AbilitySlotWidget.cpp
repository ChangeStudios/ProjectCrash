// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/Image.h"

void UAbilitySlotWidget::BindSlotToAbility(UGameplayAbility* Ability, UCrashAbilitySystemComponent* OwningASC)
{
	if (!Ability)
	{
		return;
	}

	BoundAbility = Ability;
	BoundASC = OwningASC;

	if (UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
	{
		// Update the ability icon.
		if (AbilityIcon)
		{
			AbilityIcon->SetBrushFromTexture(CrashAbility->AbilityIcon, true);
		}

		// Bind to the ability's cooldown.
		CrashAbility->AbilityCooldownStartedDelegate.AddDynamic(this, &UAbilitySlotWidget::OnCooldownStarted);

		// Bind to when this ability is activated and ended.
		CrashAbility->AbilityActivatedDelegate.AddDynamic(this, &UAbilitySlotWidget::K2_OnAbilityActivated);
		CrashAbility->AbilityEndedDelegate.AddDynamic(this, &UAbilitySlotWidget::K2_OnAbilityEnded);

		/* Bind to when an ability fails to activate. We'll check in the callback if the ability that failed is the one
		 * bound to this widget. */
		if (OwningASC)
		{
			OwningASC->AbilityFailedCallbacks.AddUObject(this, &UAbilitySlotWidget::OnAbilityFailed);
		}
	}
}

void UAbilitySlotWidget::OnCooldownStarted(const FActiveGameplayEffect& CooldownGameplayEffect)
{
	// Call this widget's blueprint logic for when the cooldown starts.
	K2_OnCooldownStarted(CooldownGameplayEffect.GetDuration());
}

void UAbilitySlotWidget::OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	// Cast to UCrashGameplayAbilityBase to retrieve the CDO.
	const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability);

	// Call this widget's blueprint logic for when its ability fails to activate, if this widget's ability failed.
	if (CrashAbility && CrashAbility->GetAbilityCDO() == BoundAbility)
	{
		K2_OnAbilityFailed(Ability, FailureReason);
	}
}
