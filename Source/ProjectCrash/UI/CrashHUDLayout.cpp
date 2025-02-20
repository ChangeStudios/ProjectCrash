// Copyright Samuel Reitich. All rights reserved.


#include "UI/CrashHUDLayout.h"

#include "CommonUIExtensions.h"
#include "CrashGameplayTags.h"
#include "Input/CommonUIInputTypes.h"

UCrashHUDLayout::UCrashHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCrashHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	/**
	 * NOTE: We're using the Common UI input framework, rather than the enhanced input framework we use for the rest of
	 * the game, for UI input because it offers a significantly simpler and compartmentalized method for these input
	 * actions. Unnecessary advanced features (e.g. "hold to go back") are not worth the trouble of having to manage
	 * additional enhanced input actions, as well as the contexts needed to bind those actions.
	 *
	 * The only reason we're allowed to mix these two input systems is because we've made a minor change to the common
	 * UI code. In the FUIActionBinding::TryCreate function, the function is supposed to fail if it isn't given an input
	 * action while enhanced input is enabled. The function works by trying to bind using an input action, then using an
	 * action tag, etc., until one works. If enhanced input is enabled, the original code will only allow binding using
	 * an input action. Even if it's given the data it needs to bind with an action tag or table row, it won't even try
	 * if we're using enhanced input in the project. We've modified this code so that even if enhanced input is enabled
	 * and an input action isn't given, it will still attempt to bind input if it's given a valid action tag or action
	 * table row.
	 *
	 * The reason we even need bEnableEnhancedInputSupport set to true in the Common UI config is because we use
	 * enhanced input actions to create bindings in our common action widget. By default, the action widget is meant to
	 * use the Common UI's action table system. If we want it to use enhanced input actions instead (since that's what
	 * our project uses for gameplay input), bEnableEnhancedInputSupport needs to be true.
	 *
	 * Note that this code change is also required for the UCommonActivatableWidget's "back handler" system to work.
	 */

	// Register the "escape" action.
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(CrashGameplayTags::TAG_UI_Action_Escape), false, FSimpleDelegate::CreateUObject(this, &UCrashHUDLayout::HandleEscapeAction)));
}

void UCrashHUDLayout::HandleEscapeAction()
{
	// Push the escape menu.
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), CrashGameplayTags::TAG_UI_Layer_Menu, EscapeMenuClass);
	}
}
