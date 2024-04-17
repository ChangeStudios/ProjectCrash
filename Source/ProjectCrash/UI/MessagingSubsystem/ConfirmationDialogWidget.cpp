// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/ConfirmationDialogWidget.h"

#include "CommonButtonBase.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"
#include "UI/Foundation/CrashButtonBase.h"

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

	for (const FDialogAction& Action : Definition->ButtonActions)
	{
		// Set up the dialog response buttons.
		UCrashButtonBase* Button = EntryBox_Buttons->CreateEntry<UCrashButtonBase>();
		Button->SetButtonText(Action.OptionalDisplayText);

		// When a response is clicked, close this widget and execute the selected response's callback.
		Button->OnClicked().AddLambda([this, Action]
		{
			DeactivateWidget();
			OnResultCallback.ExecuteIfBound(Action.Result);
		});

		// If desired, we can automatically bind Common UI input actions to trigger the corresponding button.
		if (Definition->bBindInputActions)
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

			Button->SetTriggeringInputAction(ActionRow);
		}
	}

	// Bind the result callback.
	OnResultCallback = ResultCallback;
}