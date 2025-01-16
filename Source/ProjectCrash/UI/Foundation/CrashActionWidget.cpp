// Copyright Samuel Reitich. All rights reserved.

#include "UI/Foundation/CrashActionWidget.h"

#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"

FSlateBrush UCrashActionWidget::GetIcon() const
{
	if (IsDesignTime())
	{
		if (DesignTimeKey.IsValid())
		{
			return KeyNameBackground;
		}
	}

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
				/* For keys without a defined brush (e.g. keyboard keys), use a global key background. The widget should
				 * define a text block to display the key's name on top of this, set in UpdateActionWidget. */
				else if (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::MouseAndKeyboard)
				{
					return KeyNameBackground;
				}
			}
		}
	}

	return Super::GetIcon();
}

void UCrashActionWidget::UpdateActionWidget()
{
	Super::UpdateActionWidget();

	if (KeyDisplayNameWidget)
	{
		if (IsDesignTime())
		{
			KeyDisplayNameWidget->SetText(DesignTimeKey.GetDisplayName(false));
		}
		else
		{
			KeyDisplayNameWidget->SetText(GetKeyDisplayName());
		}
	}

	// When using the key name background, sync the background's size to the key's name.
	if (!KeyDisplayNameWidget->GetText().IsEmpty())
	{
		// Desired size takes a tick to update after we change the text
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]
		{
			Icon.SetImageSize(GetParent()->GetDesiredSize());
		});
	}
}

FText UCrashActionWidget::GetKeyDisplayName()
{
	if (EnhancedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);
			FSlateBrush SlateBrush;

			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
			if (!BoundKeys.IsEmpty() && CommonInputSubsystem)
			{
				// For specified glyphs (mouse buttons, gamepad buttons, etc.), use the predefined brush instead of key text.
				for (FKey Key : BoundKeys)
				{
					if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
					{
						return FText();
					}
				}

				// For anything without a defined brush, use the key's name.
				for (FKey Key : BoundKeys)
				{
					return Key.GetDisplayName(false);
				}
			}
		}
	}

	return FText();
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
