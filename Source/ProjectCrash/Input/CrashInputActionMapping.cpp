// Copyright Samuel Reitich 2024.

#include "CrashInputActionMapping.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Mouse, "InputTag.Look.Mouse", "Input tag to activate the look action with a mouse.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Stick, "InputTag.Look.Stick", "Input tag to activate the look action with a gamepad.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Move, "InputTag.Move", "Input tag to activate the move action.");

const UInputAction* UCrashInputActionMapping::FindsInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// Iterate through every input action in this action mapping to find one mapped with the specified input tag.
	for (const FCrashInputAction& Action : InputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	// If requested, log an error specifying which input tag couldn't be found.
	if (bLogNotFound)
	{
		UE_LOG(LogInput, Warning, TEXT("Failed to find an input action mapped with the InputTag \"%s\" in the action set \"%s.\""), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
