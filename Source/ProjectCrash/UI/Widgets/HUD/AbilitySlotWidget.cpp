// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UAbilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BoundAbility = nullptr;

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

	// Update the ability icon.
	if (AbilityIcon)
	{
		if (const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
		{
			AbilityIcon->SetBrushFromTexture(CrashAbility->AbilityIcon, true);
		}
	}

	// Bind to the ability's cooldown.

	// TODO: Bind to ability's CanBeActivated
}
