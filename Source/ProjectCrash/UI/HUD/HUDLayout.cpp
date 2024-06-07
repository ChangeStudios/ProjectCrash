// Copyright Samuel Reitich 2024.


#include "UI/HUD/HUDLayout.h"

#include "AbilitySystem/CrashGameplayTags.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "Player/PlayerControllers/CrashPlayerControllerBase.h"
#include "UI/Foundation/CrashBoundActionButton.h"

void UHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Register the "escape" action.
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::TryConvert(CrashGameplayTags::TAG_UI_Action_Escape), false, FSimpleDelegate::CreateWeakLambda(this, [this] // TODO: FUIActionTag::ConvertChecked call is causing a crash.
	{
		// Push the escape menu to the "Menu" layer when the "escape" action is activated.
		if (ensure(EscapeMenuClass))
		{
			GetOwningCrashPlayer()->PushWidgetToLayer(EscapeMenuClass, CrashGameplayTags::TAG_UI_Layer_Menu);
		}
	})));
}