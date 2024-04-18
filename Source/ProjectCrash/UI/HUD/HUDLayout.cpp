// Copyright Samuel Reitich 2024.


#include "UI/HUD/HUDLayout.h"

#include "AbilitySystem/CrashGameplayTags.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "Player/PlayerControllers/CrashPlayerControllerBase.h"
#include "UI/Foundation/CrashBoundActionButton.h"

/** Action tags have to be declared statically. */
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_Action_Escape, "UI.Action.Escape")

void UHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Register the "escape" action.
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(TAG_UI_Action_Escape), false, FSimpleDelegate::CreateWeakLambda(this, [this]
	{
		// Push the escape menu to the "Menu" layer when the "escape" action is activated.
		if (ensure(EscapeMenuClass))
		{
			GetOwningCrashPlayer()->PushWidgetToLayer(EscapeMenuClass, CrashGameplayTags::TAG_UI_Layer_Menu);
		}
	})));
}