// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "Components/Image.h"

void UAbilitySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BoundAbility = nullptr;
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

	// TODO: Bind to ability's cooldown

	// TODO: Bind to ability's CanBeActivated
}
