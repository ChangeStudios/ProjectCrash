// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/HUDLayout.h"

#include "CrashGameplayTags.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "UI/Foundation/CrashBoundActionButton.h"

void UHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Register the "escape" action.
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::TryConvert(CrashGameplayTags::TAG_UI_Action_Escape), false, FSimpleDelegate::CreateWeakLambda(this, [this] // TODO: FUIActionTag::ConvertChecked call is causing a crash.
	{
		// TODO: Push the escape menu to the "Menu" layer when the "escape" action is activated.
		if (ensure(EscapeMenuClass))
		{
		}
	})));
}