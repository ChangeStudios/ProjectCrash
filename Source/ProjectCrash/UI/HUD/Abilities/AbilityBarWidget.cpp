// Copyright Samuel Reitich 2024.


#include "UI/HUD/Abilities/AbilityBarWidget.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/ChallengerBase.h"
#include "Components/DynamicEntryBox.h"
#include "Input/CrashInputComponent.h"
#include "UI/Widgets/HUD/AbilitySlotWidget.h"

void UAbilityBarWidget::OnASCReady()
{
	Super::OnASCReady();

	// Bind to relevant ASC events.
	OwningASC->InitDelegate.AddDynamic(this, &UAbilityBarWidget::OnASCInit);
	OwningASC->DeathEventDelegate.AddDynamic(this, &UAbilityBarWidget::OnDeath);
	OwningASC->AbilityGrantedDelegate.AddDynamic(this, &UAbilityBarWidget::InitializeAbilityWithUI);

	// Initialize this widget with each of its owner's current abilities.
	for (FGameplayAbilitySpec AbilitySpec : OwningASC->GetActivatableAbilities())
	{
		InitializeAbilityWithUI(AbilitySpec);
	}
}

void UAbilityBarWidget::OnASCInit(AActor* OwnerActor, AActor* AvatarActor)
{
	// Re-activate this widget when its ASC is initialized (e.g. when the owning player respawns).
	ActivateWidget();
}

void UAbilityBarWidget::OnDeath(const FDeathData& DeathData)
{
	// Deactivate this widget when its owner dies.
	DeactivateWidget();
}

void UAbilityBarWidget::InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	const UInputAction* AbilityInputAction = nullptr;

	// Find the input action associated with the given ability.
	if (AChallengerBase* OwningChallenger = Cast<AChallengerBase>(OwningASC->GetAvatarActor()))
	{
		/* If this widget's owning player has already initialized their input component, search its bound action
		 * mappings for an input action with a matching input tag. */
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
		/* If this widget's owning player's input component hasn't been initialized yet, bind a callback to call this
		 * function again once the input component is ready. */
		else
		{
			OwningChallenger->InputComponentInitializedDelegate.AddWeakLambda(this, [this, AbilitySpec]
			{
				InitializeAbilityWithUI(AbilitySpec);
			});

			return;
		}
	}

	// Decide which widget type to create for this ability: generic, equipment, or weapon.
	const FGameplayTagContainer AbilityTags = AbilitySpec.Ability->AbilityTags;

	// Create a new ability slot widget if this ability should get one.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_Slotted_Generic))
	{
		if (!AbilitySlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UAbilitySlotWidget* NewAbilitySlotWidget = AbilitySlotBox->CreateEntry<UAbilitySlotWidget>())
			{
				// Bind the widget to its ability.
				NewAbilitySlotWidget->BindSlotToAbility(AbilitySpec.Ability, AbilityInputAction, OwningASC);

				// Cache the new widget to be able to delete it later.
				AbilitySlotWidgets.Add(AbilitySpec.Ability, NewAbilitySlotWidget);

			}
		}

		return;
	}

	// Create a new equipment slot widget if this ability should get one.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_Slotted_Equipment))
	{
		if (!EquipmentSlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UAbilitySlotWidget* NewEquipmentSlotWidget = EquipmentSlotBox->CreateEntry<UAbilitySlotWidget>())
			{
				// Bind the widget to its ability.
				NewEquipmentSlotWidget->BindSlotToAbility(AbilitySpec.Ability, AbilityInputAction, OwningASC);

				// Cache the new widget to be able to delete it later.
				EquipmentSlotWidgets.Add(AbilitySpec.Ability, NewEquipmentSlotWidget);
			}
		}

		return;
	}

	// Create a new weapon slot widget if this ability should get one.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_Slotted_Weapon))
	{
		if (!WeaponSlotWidgets.Contains(AbilitySpec.Ability))
		{
			if (UAbilitySlotWidget* NewWeaponSlotWidget = WeaponSlotBox->CreateEntry<UAbilitySlotWidget>())
			{
				// Bind the widget to its ability.
				NewWeaponSlotWidget->BindSlotToAbility(AbilitySpec.Ability, AbilityInputAction, OwningASC);

				// Cache the new widget to be able to delete it later.
				WeaponSlotWidgets.Add(AbilitySpec.Ability, NewWeaponSlotWidget);
			}
		}
	}
}

void UAbilityBarWidget::UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec)
{
	// Destroy any ability slot widget associated with the removed ability.
	if (AbilitySlotWidgets.Contains(AbilitySpec.Ability))
	{
		UAbilitySlotWidget* AbilitySlotWidget = AbilitySlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
		AbilitySlotWidget->RemoveFromParent();
	}

	// Destroy any equipment slot widget associated with the removed ability.
	if (EquipmentSlotWidgets.Contains(AbilitySpec.Ability))
	{
		UAbilitySlotWidget* EquipmentSlotWidget = EquipmentSlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
		EquipmentSlotWidget->RemoveFromParent();
	}

	// Destroy any weapon slot widget associated with the removed ability.
	if (WeaponSlotWidgets.Contains(AbilitySpec.Ability))
	{
		UAbilitySlotWidget* WeaponSlotWidget = WeaponSlotWidgets.FindAndRemoveChecked(AbilitySpec.Ability);
		WeaponSlotWidget->RemoveFromParent();
	}
}
