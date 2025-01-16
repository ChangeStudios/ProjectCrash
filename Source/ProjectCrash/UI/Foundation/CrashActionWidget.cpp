// Copyright Samuel Reitich. All rights reserved.

#include "UI/Foundation/CrashActionWidget.h"

#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

FSlateBrush UCrashActionWidget::GetIcon() const
{
	// Get the associated input action from the set input tag if we haven't yet. 
	if (EnhancedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);
			FSlateBrush SlateBrush;

			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
			if (!BoundKeys.IsEmpty() && CommonInputSubsystem)
			{
				// For specified glyphs (mouse buttons, gamepad buttons, etc.), use the predefined brush.
				if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, BoundKeys[0], CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
				{
					return SlateBrush;
				}
				// For mouse buttons, use a placeholder brush. This makes sure the rim is still drawn, so we can use it as a background for our key text, which will be set here and retrieved with GetKeyText.
				else if (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::MouseAndKeyboard)
				{
					SlateBrush.DrawAs = ESlateBrushDrawType::Image;
					SlateBrush.ImageSize = IconRimBrush.ImageSize;
					SlateBrush.TintColor = FLinearColor::Transparent;
					return SlateBrush;
				}
			}
		}
	}

	return Super::GetIcon();
}

FText UCrashActionWidget::GetKeyText()
{
}

UEnhancedInputLocalPlayerSubsystem* UCrashActionWidget::GetEnhancedInputSubsystem() const
{
	const UWidget* BoundWidget = DisplayedBindingHandle.GetBoundWidget();
	if (const ULocalPlayer* BindingOwner = BoundWidget ? BoundWidget->GetOwningLocalPlayer() : GetOwningLocalPlayer())
	{
		return BindingOwner->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	return nullptr;
}
