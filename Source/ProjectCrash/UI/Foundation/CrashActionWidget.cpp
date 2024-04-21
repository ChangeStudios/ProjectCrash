// Copyright Samuel Reitich 2024.


#include "UI/Foundation/CrashActionWidget.h"

#include "CommonInputBaseTypes.h"
#include "CommonInputSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "CommonTextBlock.h"

FKeyOverrideRow* FKeyOverrideRow::GetRowFromKey(FKey InKey, UDataTable* InDataTable)
{
	// Ensure the given data table has the correct structure.
	check(InDataTable);
	check(InDataTable->GetRowStruct() == FKeyOverrideRow::StaticStruct());

	// Iterate through each row, searching for one with a matching Key value.
	TArray<FTableRowBase*> Rows;
	InDataTable->GetAllRows("", Rows);
	for (FTableRowBase* Row : Rows)
	{
		// This feels wrong but it works.
		FKeyOverrideRow* KeyRow = (FKeyOverrideRow*)Row;
		if (KeyRow->Key == InKey)
		{
			return KeyRow;
		}
	}

	return nullptr;
}

FSlateBrush UCrashActionWidget::GetIcon() const
{
	// Search for an input brush for any of the keys bound to the associated input action.
	if (AssociatedInputAction)
	{
		const UCommonInputSubsystem* CommonInputSubsystem = GetInputSubsystem();
		const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = GetEnhancedInputSubsystem();
		TArray<FKey> BoundKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(AssociatedInputAction);
		FSlateBrush SlateBrush;

		for (const FKey Key : BoundKeys)
		{
			// If the Common input data has a brush defined for this key, use that brush.
			if (UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, CommonInputSubsystem->GetCurrentInputType(), CommonInputSubsystem->GetCurrentGamepadName()))
			{
				KeyText->SetText(FText());
				return SlateBrush;
			}

			// For MnK controls without defined brushes, create a dynamic icon using their name.
			if (CommonInputSubsystem->GetCurrentInputType() == ECommonInputType::MouseAndKeyboard)
			{
				// By default, use the engine's short display name. This is correct for most keys.
				FText KeyName = Key.GetDisplayName(false);

				/* If we have a name override defined in our key override table (e.g. "LShift" instead of "Left Shift,"
				 * use that name instead. */
				if (KeyNameOverridesTable)
				{
					if (FKeyOverrideRow* KeyRow = FKeyOverrideRow::GetRowFromKey(Key, KeyNameOverridesTable))
					{
						KeyName = KeyRow->DisplayKeyName;
					}
				}

				KeyText->SetText(KeyName);
				return DynamicIconBrush;
			}
		}
	}

	// Fall back to the Common UI universal action data table if no brush could be found for any bound keys.
	return Super::GetIcon();
}

UEnhancedInputLocalPlayerSubsystem* UCrashActionWidget::GetEnhancedInputSubsystem() const
{
	// Retrieve this widget's bound widget's local player, if it has one. Otherwise, use this widget's owner.
	const UWidget* BoundWidget = DisplayedBindingHandle.GetBoundWidget();
	const ULocalPlayer* BindingOwner = BoundWidget ? BoundWidget->GetOwningLocalPlayer() : GetOwningLocalPlayer();

	// Retrieve the local subsystem from the local player.
	return BindingOwner->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
