// Copyright Samuel Reitich. All rights reserved.


#include "CrashInputComponent.h"

#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "CrashInputActionMapping.h"
#include "GameplayAbilitySpec.h"

void UCrashInputComponent::BindAbilityInputActions(const UCrashInputActionMapping* ActionMapping)
{
	check(ActionMapping);

	// Cache the new action mapping.
	CurrentActionMappings.AddUnique(ActionMapping);

	// Bind each ability input action in the given mapping to the ability handler functions using their input tags.
	for (const FCrashInputAction& AbilityInputAction : ActionMapping->AbilityInputActions)
	{
		BindAction(AbilityInputAction.InputAction, ETriggerEvent::Triggered, this, &ThisClass::Input_AbilityInputTagPressed, AbilityInputAction.InputTag);
		BindAction(AbilityInputAction.InputAction, ETriggerEvent::Completed, this, &ThisClass::Input_AbilityInputTagReleased, AbilityInputAction.InputTag);
	}
}

void UCrashInputComponent::RemoveAbilityInputActions(const UCrashInputActionMapping* ActionMapping)
{
	check(ActionMapping);

	// Only unbind the given actions if they are currently bound.
	if (ensureAlwaysMsgf(CurrentActionMappings.Contains(ActionMapping), TEXT("Attempted to unbind abilities in action mapping [%s] from pawn [%s], but the abilities are now bound."), *GetNameSafe(ActionMapping), *GetNameSafe(GetOwner())))
	{
		// Unbind each ability input action in the given mapping.
		for (const TUniquePtr<FEnhancedInputActionEventBinding>& ActionBinding : GetActionEventBindings())
		{
			if (ActionMapping->AbilityInputActions.Contains(ActionBinding->GetAction()))
			{
				RemoveBinding(*ActionBinding.Get());
			}
		}

		// Remove the cached action mapping.
		CurrentActionMappings.Remove(ActionMapping);
	}
}

void UCrashInputComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	// Notify the owner's ASC that an ability input was pressed.
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		CrashASC->AbilityInputTagPressed(InputTag);
	}
}

void UCrashInputComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	// Notify the owner's ASC that an ability input was released.
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		CrashASC->AbilityInputTagReleased(InputTag);
	}
}
