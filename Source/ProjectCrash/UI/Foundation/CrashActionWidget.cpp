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
	/* In the editor, use a design-time brush to preview what that brush will look like in this widget. If a design-time
	 * key is set, hide this widget to preview what it would look like with that key in a dynamic icon instead. */
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

	if (EnhancedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			// Get the keys bound to this widget's associated input action. 
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);
			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();

			if (!BoundKeys.IsEmpty() && CommonInputSubsystem)
			{
				FSlateBrush SlateBrush;

				/* Use the defined brush of the first bound key that has one. This is set in "Controller Data" classes,
				 * and usually used for gamepad and mouse keys. */
				for (FKey Key : BoundKeys)
				{
					if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
					{
						return SlateBrush;
					}
				}

				/* If a key doesn't have a defined brush (e.g. any keyboard keys), hide this widget. We'll use the
				 * dynamic key icon widgets instead, in UpdateDynamicKeyIcon. */
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
	UpdateDynamicKeyIcon();
}

void UCrashActionWidget::UpdateDynamicKeyIcon()
{
	const FText KeyDisplayName = GetKeyDisplayName(); 
	const bool bUsingDynamicKeyIcon = !KeyDisplayName.IsEmpty();
	const ESlateVisibility DynamicKeyIconVisibility = (bUsingDynamicKeyIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (DynamicKeyIconDisplayName)
	{
		// Update the key's displayed name. If we're using a predefined brush, clear it instead.
		DynamicKeyIconDisplayName->SetVisibility(DynamicKeyIconVisibility);
		DynamicKeyIconDisplayName->SetText(bUsingDynamicKeyIcon ? GetKeyDisplayName() : FText::GetEmpty());
	}

	// Toggle dynamic key icon background visibility if we're using it.
	if (DynamicKeyIconBackground)
	{
		DynamicKeyIconBackground->SetVisibility(DynamicKeyIconVisibility);
	}

	// Hide this widget if we're using a dynamic key icon instead.
	SetVisibility(bUsingDynamicKeyIcon ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

FText UCrashActionWidget::GetKeyDisplayName() const
{
#if WITH_EDITORONLY_DATA
	// In the editor, use a design-time key to preview what that key will look like in this widget.
	if (IsDesignTime())
	{
		if (DesignTimeKey.IsValid())
		{
			return DesignTimeKey.GetDisplayName(false);
		}
		else
		{
			return FText::GetEmpty();
		}
	}
#endif

	// NOTE: This is double work. We do this same thing in GetIcon, but we can't cache anything because it's const.
	if (EnhancedInputAction)
	{
		if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem())
		{
			TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(EnhancedInputAction);
			const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();

			if (!BoundKeys.IsEmpty() && CommonInputSubsystem)
			{
				FSlateBrush SlateBrush;

				// Don't use the key's name if the key has a predefined brush.
				for (FKey Key : BoundKeys)
				{
					if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
					{
						return FText::GetEmpty();
					}
				}

				// If there are no bound keys with a defined brush, use the first key's short display name.
				for (FKey Key : BoundKeys)
				{
					return Key.GetDisplayName(false);
				}
			}
		}
	}

	return FText::GetEmpty();
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
