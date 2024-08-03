// Copyright Samuel Reitich. All rights reserved.


#include "UI/Deprecated/CrashButtonBase.h"

void UCrashButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Initialize this button's style and text with its internal properties.
	UpdateButtonStyle();
	RefreshButtonText();
}

void UCrashButtonBase::SetButtonText(const FText& InText)
{
	// Update this button's internal text property.
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

void UCrashButtonBase::RefreshButtonText()
{
	// Update this button's displayed text with the current value of its ButtonText property.
	UpdateButtonText(ButtonText);
}
