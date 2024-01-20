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
						ASC->TryActivateAbility(AbilitySpec.Handle);
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
						/* Broadcast that the input for this ability was released. This is commonly used to end
						 * abilities when their input is released, such as an automatic shooting ability. */
						ASC->AbilitySpecInputReleased(const_cast<FGameplayAbilitySpec&>(AbilitySpec));
					}
				}
			}
		}
	}
}
