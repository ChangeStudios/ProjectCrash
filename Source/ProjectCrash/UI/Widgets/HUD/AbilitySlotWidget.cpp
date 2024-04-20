// Copyright Samuel Reitich 2024.


#include "UI/Widgets/HUD/AbilitySlotWidget.h"

#include "CommonInputSubsystem.h"
#include "InputAction.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/Image.h"
#include "UI/Foundation/CrashActionWidget.h"

void UAbilitySlotWidget::BindSlotToAbility(UGameplayAbility* Ability, const UInputAction* InputAction, UCrashAbilitySystemComponent* OwningASC)
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
	}

	InputActionWidget->AssociatedInputAction = InputAction;
}
