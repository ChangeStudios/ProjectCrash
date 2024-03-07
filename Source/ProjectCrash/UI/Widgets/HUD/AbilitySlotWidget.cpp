// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UAbilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Instead of hiding the cooldown bar when the ability is not on a cooldown, we can just set its progress to 0.
	CooldownProgressBar->SetPercent(0.0f);
}

void UAbilitySlotWidget::BindSlotToAbility(UGameplayAbility* Ability)
{
	if (!Ability)
	{
		return;
	}

	BoundAbility = Ability;

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

		// TODO: Bind to when the ability fails to activate.
	}
}

void UAbilitySlotWidget::OnCooldownStarted(const FActiveGameplayEffect& CooldownGameplayEffect)
{
	// Call this widget's blueprint logic for when the cooldown starts.
	K2_OnCooldownStarted(CooldownGameplayEffect.GetDuration());
}
