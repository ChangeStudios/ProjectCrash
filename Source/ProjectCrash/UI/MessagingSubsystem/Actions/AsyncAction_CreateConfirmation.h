// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UI/MessagingSubsystem/CrashUIMessagingSubsystem.h"
#include "AsyncAction_CreateConfirmation.generated.h"

class UDialogDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIMessagingResultMulticastSignature, EUIMessageResult, Result);

/**
 * Creates and displays a confirmation dialog widget with a series of responses which the user can select. Provides
 * callbacks to when the user has selected a response.
 *
 * Dialogs are pushed to the "Dialog" stack of the global layered widget, if a "Dialog" stack exists.
 */
UCLASS()
class PROJECTCRASH_API UAsyncAction_CreateConfirmation : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	// Action construction.

public:

	/**
	 * Creates and displays a confirmation dialog widget with the given information.
	 *
	 * @param InWorldContextObject		World context object. Use a player controller to explicitly choose to whom the
	 *									dialog will be given, or use a widget to push to that widget's owner.
	 * @param InHeaderText				The dialog's header text.
	 * @param InBodyText				The dialog's body text.
	 * @param InButtonActions			List of actions that will be provided to the player as possible responses.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UserInterface|Actions", Meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject"))
	static UAsyncAction_CreateConfirmation* CreateConfirmationDialog(
		UObject* InWorldContextObject,
		FText InHeaderText,
		FText InBodyText,
		const TArray<FDialogAction>& InButtonActions
	);

	/** Creates and pushes the confirmation widget, to the global layered widget's "Dialog" stack. */
	virtual void Activate() override;



	// Dialog result.

public:

	/** Callback fired when the user selects a dialog response. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnResult")
	FUIMessagingResultMulticastSignature ResultDelegate;

private:

	/** Broadcasts ResultDelegate and destroys the dialog when a response is selected by the user. */
	void HandleConfirmationResult(EUIMessageResult ConfirmationResult);



	// Task data.

private:

	/** World context object. */
	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	/** Player to whom the dialog is pushed. */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	/** Definition of the dialog being created and pushed by this action. */
	UPROPERTY(Transient)
	TObjectPtr<UDialogDefinition> DialogDefinition;
};
