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

	// Set up the dialog response buttons.
	for (const FDialogAction& Action : Definition->ButtonActions)
	{
		FDataTableRowHandle ActionRow;

		// Bind each button's input action.
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

		UCrashButtonBase* Button = EntryBox_Buttons->CreateEntry<UCrashButtonBase>();
		Button->SetTriggeringInputAction(ActionRow);
		Button->SetButtonText(Action.OptionalDisplayText);

		// When a response is clicked, close this widget and execute the selected response's callback.
		Button->OnClicked().AddLambda([this, Action]
		{
			DeactivateWidget();
			OnResultCallback.ExecuteIfBound(Action.Result);
		});
	}

	// Bind the result callback.
	OnResultCallback = ResultCallback;
}