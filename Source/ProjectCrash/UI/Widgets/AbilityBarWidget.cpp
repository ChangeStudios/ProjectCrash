// Copyright Samuel Reitich 2024.


#include "UI/Widgets/AbilityBarWidget.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Runtime/UMG/Public/Components/PanelWidget.h"

void UAbilityBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(AbilitySlotWidgetClass);
	check(WeaponSlotWidgetClass);

	/** Perform an initial update to this widget with its ASC. */
	if (OwningASC)
	{
		InitializeAbilityBar();
	}
}

void UAbilityBarWidget::InitializeAbilityBar()
{
	if (!OwningASC)
	{
		ABILITY_LOG(Error, TEXT("UAbilityBarWidget: Ability bar widget could not be initialized. Could not find owning player's ASC."));
		return;
	}

	for (FGameplayAbilitySpec& ActivatableAbility : OwningASC->GetActivatableAbilities())
	{
		InitializeAbilityWithUI(ActivatableAbility);
	}
}

void UAbilityBarWidget::OnAbilityGranted(const FGameplayAbilitySpec& GrantedAbilitySpec)
{
	// Initialize the new ability with the ability bar.
	InitializeAbilityWithUI(GrantedAbilitySpec);
}

void UAbilityBarWidget::OnAbilityRemoved(const FGameplayAbilitySpec& RemovedAbilitySpec)
{
	// Uninitialize the removed ability from the ability bar.
	UninitializeAbilityWithUI(RemovedAbilitySpec);
}

void UAbilityBarWidget::InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	const FGameplayTagContainer AbilityTags = AbilitySpec.Ability->AbilityTags;

	// Create a new ability slot widget if this ability should get one. 
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_AbilityBar))
	{
		if (!AbilitySlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UCommonActivatableWidget* NewAbilitySlotWidget = CreateWidget<UCommonActivatableWidget>(GetOwningPlayer(), AbilitySlotWidgetClass))
			{
				// Add the new widget to the ability slot container.
				AbilitySlotPanel->AddChild(NewAbilitySlotWidget);
				
				// Cache the new widget to be able to delete it later.
				AbilitySlotWidgets.Add(AbilitySpec.Ability, NewAbilitySlotWidget);
			}
		}
	}

	// Create a new weapon slot widget if this ability should get one.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_Weapon))
	{
		if (!WeaponSlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UCommonActivatableWidget* NewWeaponSlotWidget = CreateWidget<UCommonActivatableWidget>(GetOwningPlayer(), WeaponSlotWidgetClass))
			{
				// Add the new widget to the weapon slot container.
				WeaponSlotPanel->AddChild(NewWeaponSlotWidget);

				// Cache the new widget to be able to delete it later.
				WeaponSlotWidgets.Add(AbilitySpec.Ability, NewWeaponSlotWidget);
			}
		}
	}
}

void UAbilityBarWidget::UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	// Destroy the ability slot widget associated with the removed ability.
	if (AbilitySlotWidgets.Contains(AbilitySpec.Ability))
	{
		UCommonActivatableWidget* AbilitySlotWidget = AbilitySlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
		AbilitySlotWidget->RemoveFromParent();
	}

	// Destroy the weapon slot widget associated with the removed ability.
	if (WeaponSlotWidgets.Contains(AbilitySpec.Ability))
	{
		UCommonActivatableWidget* WeaponSlotWidget = WeaponSlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
		WeaponSlotWidget->RemoveFromParent();
	}
}
