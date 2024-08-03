// Copyright Samuel Reitich. All rights reserved.


#include "UI/Deprecated/CrashBoundActionButton.h"

#include "CommonInputSubsystem.h"

void UCrashBoundActionButton::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind callback to when this widget's owning player's input method changes.
	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);

		// Initializing call.
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void UCrashBoundActionButton::NativeDestruct()
{
	Super::NativeDestruct();

	// Unbind callbacks.
	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->OnInputMethodChangedNative.RemoveAll(this);
	}
}

void UCrashBoundActionButton::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	// Switch to the desired style for the given input method, if it has been defined.
	TSubclassOf<UCommonButtonStyle> NewStyle = nullptr;

	if (NewInputMethod == ECommonInputType::Gamepad)
	{
		if (GamepadStyle)
		{
			NewStyle = GamepadStyle;
		}
	}
	else if (KeyboardStyle)
	{
		NewStyle = KeyboardStyle;
	}

	if (NewStyle)
	{
		SetStyle(NewStyle);
	}

	// Call optional blueprint logic.
	K2_OnInputMethodChanged(NewInputMethod);
}
