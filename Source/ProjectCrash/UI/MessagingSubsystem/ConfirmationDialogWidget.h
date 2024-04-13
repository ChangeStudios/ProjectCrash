// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "UI/MessagingSubsystem/DialogWidget.h"
#include "ConfirmationDialogWidget.generated.h"

class UCommonTextBlock;
class UCommonRichTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTCRASH_API UConfirmationDialogWidget : public UDialogWidget
{
	GENERATED_BODY()

public:

	virtual void SetupDialog(UDialogDefinition* Definition, FUIMessagingResultSignature ResultCallback) override;

protected:

	virtual void CloseConfirmationWindow(EUIMessageResult Result);

private:

	FUIMessagingResultSignature OnResultCallback;

protected:

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle CancelAction;
};
