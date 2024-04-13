// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/ConfirmationDialogWidget.h"

#include "CommonButtonBase.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"

void UConfirmationDialogWidget::SetupDialog(UDialogDefinition* Definition, FUIMessagingResultSignature ResultCallback)
{
	Super::SetupDialog(Definition, ResultCallback);

	// Update the displayed dialog text.
	Text_Title->SetText(Definition->HeaderText);
	RichText_Description->SetText(Definition->BodyText);

	// Clear any default buttons the dialog has.
	EntryBox_Buttons->Reset<UCommonButtonBase>([](UCommonButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	// Set up the dialog response buttons.
	for (const FDialogAction& Action : Definition->ButtonActions)
	{
		FDataTableRowHandle ActionRow;

		switch (Action.Result)
		{
			case EUIMessageResult::Confirmed:
			{
				ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
				break;
			}
			case EUIMessageResult::Declined:
			{
				ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
				break;
			}
			case EUIMessageResult::Cancelled:
			{
				ActionRow = CancelAction;
				break;
			}
			default:
			{
				ensure(false);
				continue;
			}
		}

		UCommonButtonBase* Button = EntryBox_Buttons->CreateEntry<UCommonButtonBase>();
		Button->SetTriggeringInputAction(ActionRow);
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);

		// TODO: Update button text.
	}

	// Bind the result callback.
	OnResultCallback = ResultCallback;
}

void UConfirmationDialogWidget::CloseConfirmationWindow(EUIMessageResult Result)
{
	// Close this confirmation dialog by deactivating it.
	DeactivateWidget();

	// Execute the dialog result callback.
	OnResultCallback.ExecuteIfBound(Result);
}
