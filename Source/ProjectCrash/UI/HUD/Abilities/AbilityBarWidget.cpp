// Copyright Samuel Reitich 2024.


#include "UI/HUD/Abilities/AbilityBarWidget.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/ChallengerBase.h"
#include "Characters/ChallengerData.h"
#include "Components/DynamicEntryBox.h"
#include "Input/CrashInputComponent.h"
#include "UI/Widgets/HUD/AbilitySlotWidget.h"

void UAbilityBarWidget::OnASCReady()
{
	Super::OnASCReady();

	OwningASC->InitDelegate.AddDynamic(this, &UAbilityBarWidget::OnASCInit);
	OwningASC->DeathEventDelegate.AddDynamic(this, &UAbilityBarWidget::OnDeath);

	for (FGameplayAbilitySpec AbilitySpec : OwningASC->GetActivatableAbilities())
	{
		InitializeAbilityWithUI(AbilitySpec);
	}

	OwningASC->AbilityGrantedDelegate.AddDynamic(this, &UAbilityBarWidget::InitializeAbilityWithUI);
}

void UAbilityBarWidget::OnASCInit(AActor* OwnerActor, AActor* AvatarActor)
{
	ActivateWidget();
}

void UAbilityBarWidget::OnDeath(const FDeathData& DeathData)
{
	DeactivateWidget();
}

void UAbilityBarWidget::InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	const UInputAction* AbilityInputAction = nullptr;

	if (AChallengerBase* OwningChallenger = Cast<AChallengerBase>(OwningASC->GetAvatarActor()))
	{
		if (OwningChallenger->GetCrashInputComponent())
		{
			if (const UCrashInputComponent* CrashInputComponent = OwningChallenger->GetCrashInputComponent())
			{
				for (const UCrashInputActionMapping* ActionMapping : CrashInputComponent->GetCurrentActionMappings())
				{
					const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability);
					if (CrashAbility && CrashAbility->GetInputTag().IsValid() && ActionMapping->FindAbilityInputActionForTag(CrashAbility->GetInputTag()))
					{
						AbilityInputAction = ActionMapping->FindAbilityInputActionForTag(CrashAbility->GetInputTag());
					}
				}
			}
		}
		else
		{
			OwningChallenger->InputComponentInitializedDelegate.AddWeakLambda(this, [this, AbilitySpec]
			{
				InitializeAbilityWithUI(AbilitySpec);
			});

			return;
		}
	}

	const FGameplayTagContainer AbilityTags = AbilitySpec.Ability->AbilityTags;

	// Create a new ability slot widget if this ability should get one. 
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_AbilitySlot))
	{
		if (!AbilitySlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UAbilitySlotWidget* NewAbilitySlotWidget = AbilitySlotPanel->CreateEntry<UAbilitySlotWidget>())
			{
				// Bind the widget to its ability.
				NewAbilitySlotWidget->BindSlotToAbility(AbilitySpec.Ability, AbilityInputAction, OwningASC);

				// Cache the new widget to be able to delete it later.
				AbilitySlotWidgets.Add(AbilitySpec.Ability, NewAbilitySlotWidget);
			}
		}
	}

	// // Create a new weapon slot widget if this ability should get one.
	// if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_WeaponSlot))
	// {
	// 	if (!WeaponSlotWidgets.Contains(AbilitySpec.Ability))
	// 	{
	// 		if (UCommonActivatableWidget* NewWeaponSlotWidget = CreateWidget<UCommonActivatableWidget>(GetOwningPlayer(), WeaponSlotWidgetClass))
	// 		{
	// 			// Add the new widget to the weapon slot container.
	// 			WeaponSlotPanel->AddChild(NewWeaponSlotWidget);
	//
	// 			// Cache the new widget to be able to delete it later.
	// 			WeaponSlotWidgets.Add(AbilitySpec.Ability, NewWeaponSlotWidget);
	// 		}
	// 	}
	// }
}

void UAbilityBarWidget::UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	// // Destroy the ability slot widget associated with the removed ability.
	// if (AbilitySlotWidgets.Contains(AbilitySpec.Ability))
	// {
	// 	UCommonActivatableWidget* AbilitySlotWidget = AbilitySlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
	// 	AbilitySlotWidget->RemoveFromParent();
	// }
	//
	// // Destroy the weapon slot widget associated with the removed ability.
	// if (WeaponSlotWidgets.Contains(AbilitySpec.Ability))
	// {
	// 	UCommonActivatableWidget* WeaponSlotWidget = WeaponSlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
	// 	WeaponSlotWidget->RemoveFromParent();
	// }
}
