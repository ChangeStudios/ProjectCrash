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
