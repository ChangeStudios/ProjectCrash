// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/MessagingSubsystem/DialogWidget.h"
#include "ConfirmationDialogWidget.generated.h"

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

/**
 * A dialog prompt widget that provides the user with a set of response options which close the dialog and fire an
 * optional callback.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTCRASH_API UConfirmationDialogWidget : public UDialogWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Populates the dialog content (its title, description, and buttons). */
	virtual void SetupDialog(UDialogDefinition* Definition, FUIMessagingResultSignature ResultCallback) override;



	// Widgets.

protected:

	/** The dialog title text widget. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	/** The dialog description text widget. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	/** The list of the dialog's response buttons. */
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;



	// Response callbacks.

private:

	/** The callback that will be broadcast when the user selects a dialog response. */
	FUIMessagingResultSignature OnResultCallback;



	// Response actions.

protected:

	/** The action used to execute the "cancel" response to the dialog. */
	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle CancelAction;
};
