// Copyright Samuel Reitich. All rights reserved.

#include "UI/Foundation/CrashActionWidget.h"

#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "EnhancedInputSubsystems.h"
#include "TimerManager.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Engine/LocalPlayer.h"

FSlateBrush UCrashActionWidget::GetIcon() const
{
#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		if (DesignTimeKey.IsValid())
		{
			FSlateBrush SlateBrush;
			SlateBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
			return SlateBrush;
		}
		else
		{
			return DesignTimeBrush;
		}
	}
#endif

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
				for (FKey Key : BoundKeys)
				{
					if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
					{
						return SlateBrush;
					}
				}

				/* For keys without a defined brush (e.g. keyboard keys), use a global key background. The widget should
				 * define a text block to display the key's name on top of this, set in UpdateActionWidget. */
				SlateBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
				return SlateBrush;
			}
		}
	}

	return Super::GetIcon();
}

void UCrashActionWidget::UpdateActionWidget()
{
	Super::UpdateActionWidget();

	UpdateKeyName();
}

void UCrashActionWidget::UpdateKeyName()
{
	if (KeyDisplayNameWidget)
	{
		FText KeyName = GetKeyDisplayName();
		bool bUsingKeyName = !KeyName.IsEmpty();

		// Update the key's displayed name
		KeyDisplayNameWidget->SetText(KeyName);

#if WITH_EDITORONLY_DATA
		if (IsDesignTime() && DesignTimeKey.IsValid())
		{
			KeyDisplayNameWidget->SetText(DesignTimeKey.GetDisplayName(false));
		}
#endif

		// // When using the key name background, sync the background's size to the key's name.
		// if (!KeyDisplayNameWidget->GetText().IsEmpty())
		// {
		// 	// Desired size takes a tick to update after we change the text
		// 	GetWorld()->GetTimerManager().SetTimerForNextTick([this]
		// 	{
		// 		Icon.SetImageSize(GetParent()->GetDesiredSize());
		// 	});
		// }

		if (KeyBackgroundWidget)
		{
			ESlateVisibility BackgroundVisibility = bUsingKeyName ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
			KeyBackgroundWidget->SetVisibility(BackgroundVisibility);
		}
	}
}

FText UCrashActionWidget::GetKeyDisplayName()
{
#if WITH_EDITORONLY_DATA
	if (IsDesignTime())
	{
		if (DesignTimeKey.IsValid())
		{
			return DesignTimeKey.GetDisplayName(false);
		}
	}
#endif

	if (EnhancedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);

			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
			if (!BoundKeys.IsEmpty() && CommonInputSubsystem)
			{
				FSlateBrush SlateBrush;

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
