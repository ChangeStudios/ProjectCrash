// Copyright Samuel Reitich. All rights reserved.


#include "UI/Deprecated/HUD/Abilities/AbilitySlotWidget.h"

#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/Image.h"
#include "UI/Deprecated/CrashActionWidget.h"

void UAbilitySlotWidget::BindSlotToAbility(UGameplayAbility* Ability, const UInputAction* InputAction, UCrashAbilitySystemComponent* OwningASC)
{
	if (!Ability)
	{
		return;
	}

	// Cache widget data.
	BoundAbility = Ability;
	BoundASC = OwningASC;

	// Update the ability icon.
	if (const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
	{
		if (AbilityIcon)
		{
			// AbilityIcon->SetBrushFromTexture(CrashAbility->AbilityIcon, true);
		}
	}

	// Update the input action bound to this widget.
	InputActionWidget->AssociatedInputAction = InputAction;
}