// Copyright Samuel Reitich 2024.


#include "CrashInputComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"

void UCrashInputComponent::BindAbilityInputActions(const UCrashInputActionMapping* ActionMapping)
{
	// Bind each ability input action in the given mapping to ability handler functions using their input tags.
	for (const FCrashInputAction& AbilityInputAction : ActionMapping->AbilityInputActions)
	{
		BindAction(AbilityInputAction.InputAction, ETriggerEvent::Triggered, this, &ThisClass::Input_AbilityInputTagPressed, AbilityInputAction.InputTag);
		BindAction(AbilityInputAction.InputAction, ETriggerEvent::Completed, this, &ThisClass::Input_AbilityInputTagReleased, AbilityInputAction.InputTag);
	}
}

void UCrashInputComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	ABILITY_LOG(Verbose, TEXT("Attempting to activate ability with tag [%s]."), *InputTag.ToString());

	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		// Cache the array in case it changes during iteration.
		const TArray<FGameplayAbilitySpec> ActivatableAbilities = ASC->GetActivatableAbilities();

		// Search the ASC's list of activatable abilities for one with a matching input tag.
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities)
		{
			if (AbilitySpec.Ability)
			{
				if (UCrashGameplayAbilityBase* Ability = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability))
				{
					// Activate any ability with the matching input tag.
					if (Ability->InputTags.HasTagExact(InputTag))
					{
						switch (Ability->GetActivationStyle())
						{

						// One-off activate-on-input abilities. These are most common.
						case ECrashAbilityActivationStyle::ActivateOnInputTriggered:
							{
								ASC->TryActivateAbility(AbilitySpec.Handle);
								break;
							}

						// If the ability is being toggled, activate or cancel it depending on its current state.
						case ECrashAbilityActivationStyle::ToggleOnInputTriggered:
							{
								if (AbilitySpec.IsActive())
								{
									ASC->CancelAbility(Ability);
								}
								else
								{
									ASC->TryActivateAbility(AbilitySpec.Handle);
								}
								break;
							}

						// Activate abilities that are only activated while their input is held. They will be cancelled when the input is released.
						case ECrashAbilityActivationStyle::ActivateWhileInputHeld:
						{
							ASC->TryActivateAbility(AbilitySpec.Handle);
							break;
						}

						// No other activation styles use input to activate their abilities.
						default:
							{
								ABILITY_LOG(Warning, TEXT("Tried to activate ability %s with input, but its activation style does not use input."), *Ability->GetName());
								break;
							}
						}
					}
				}
			}
		}
	}
}

void UCrashInputComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		// Cache the array in case it changes during iteration.
		const TArray<FGameplayAbilitySpec> ActivatableAbilities = ASC->GetActivatableAbilities();

		// Search the ASC's list of activatable abilities for one with a matching input tag.
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities)
		{
			if (AbilitySpec.Ability)
			{
				if (UCrashGameplayAbilityBase* Ability = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability))
				{
					if (Ability->InputTags.HasTagExact(InputTag))
					{
						// Broadcast that the input for this ability was released.
						ASC->AbilitySpecInputReleased(const_cast<FGameplayAbilitySpec&>(AbilitySpec));

						// Cancel abilities that are only active while their input is being held.
						if (Ability->GetActivationStyle() == ECrashAbilityActivationStyle::ActivateWhileInputHeld)
						{
							ASC->CancelAbility(Ability);
						}
					}
				}
			}
		}
	}
}
