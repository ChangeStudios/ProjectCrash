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
	UE_LOG(LogTemp, Error, TEXT("C"));
	
	Super::SetupDialog(Definition, ResultCallback);

	// Update the displayed dialog text.
	Text_Title->SetText(Definition->HeaderText);
	RichText_Description->SetText(Definition->BodyText);

	UE_LOG(LogTemp, Error, TEXT("definition: %s, title: %s, action size: %i"), *Definition->BodyText.ToString(), *Definition->HeaderText.ToString(), Definition->ButtonActions.Num());

	// Clear any default buttons the dialog has.
	EntryBox_Buttons->Reset<UCommonButtonBase>([](UCommonButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});
	
	UE_LOG(LogTemp, Error, TEXT("B"));

	
	// Set up the dialog response buttons.
	for (const FDialogAction& Action : Definition->ButtonActions)
	{
		UE_LOG(LogTemp, Error, TEXT("C"));
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

		UCrashButtonBase* Button = EntryBox_Buttons->CreateEntry<UCrashButtonBase>();
		Button->SetTriggeringInputAction(ActionRow);
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
		Button->SetButtonText(Action.OptionalDisplayText);
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
