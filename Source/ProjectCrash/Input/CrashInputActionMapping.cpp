// Copyright Samuel Reitich. All rights reserved.


#include "CrashInputActionMapping.h"

const UInputAction* UCrashInputActionMapping::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	/* Iterate through every native input action in the native action mapping to find one mapped with the specified
	 * input tag. */
	for (const FCrashInputAction& NativeAction : NativeInputActions)
	{
		if (NativeAction.InputAction && NativeAction.InputTag == InputTag)
		{
			return NativeAction.InputAction;
		}
	}

	// If requested, log an error specifying which input tag couldn't be found.
	if (bLogNotFound)
	{
		UE_LOG(LogInput, Warning, TEXT("Failed to find a native input action mapped with the InputTag \"%s\" in the action set \"%s.\""), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UCrashInputActionMapping::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	/* Iterate through every ability input action in the native action mapping to find one mapped with the specified
	 * input tag. */
	for (const FCrashInputAction& AbilityAction : AbilityInputActions)
	{
		if (AbilityAction.InputAction && AbilityAction.InputTag == InputTag)
		{
			return AbilityAction.InputAction;
		}
	}

	// If requested, log an error specifying which input tag couldn't be found.
	if (bLogNotFound)
	{
		UE_LOG(LogInput, Warning, TEXT("Failed to find an ability input action mapped with the InputTag \"%s\" in the action set \"%s.\""), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
