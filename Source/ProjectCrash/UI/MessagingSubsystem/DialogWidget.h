// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CrashUIMessagingSubsystem.h"
#include "UI/Deprecated/Framework/CrashActivatableWidget.h"
#include "DialogWidget.generated.h"

/**
 * A single action that can be taken in response to a dialog, e.g. "Confirm."
 */
USTRUCT(BlueprintType)
struct FDialogAction
{
	GENERATED_BODY()

	/** The dialog option to provide. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUIMessageResult Result = EUIMessageResult::Unknown;

	/** Display text to use instead of the action name associated with the result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionalDisplayText;

	bool operator==(const FDialogAction& Other) const
	{
		return Result == Other.Result &&
			OptionalDisplayText.EqualTo(Other.OptionalDisplayText);
	}
};



/**
 * Defines the properties of a dialog. Defines the frontend information (displayed message text) and the actions that
 * can be taken in response to the dialog.
 */
UCLASS()
class PROJECTCRASH_API UDialogDefinition : public UObject
{
	GENERATED_BODY()

public:

	/** The header of the message to display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Header")
	FText HeaderText;
	
	/** The body of the message to display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Body")
	FText BodyText;

	/** If true, Common UI input actions will automatically be bound each action button in the dialog. E.g. the
	 * "Confirm" action will automatically trigger any "Confirm" button in the dialog widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBindInputActions;

	/** The actions that can be taken in response to this dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogAction> ButtonActions;
};



/**
 * Base dialog widget class. Prompts the user with a message and provides a series of actions with which to response.
 * Useful for prompts like confirmation messages or error messages.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UDialogWidget : public UCrashActivatableWidget
{
	GENERATED_BODY()

public:

	/** Called to initialize this dialog with the given data. */
	virtual void SetupDialog(UDialogDefinition* Definition, FUIMessagingResultSignature ResultCallback) {}

	/** Called to explicitly kill this dialog, destroying it without receiving user input. */
	virtual void KillDialog() {}
};
