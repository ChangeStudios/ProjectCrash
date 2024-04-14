// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/Actions/AsyncAction_CreateConfirmation.h"

#include "UI/MessagingSubsystem/DialogWidget.h"

UAsyncAction_CreateConfirmation* UAsyncAction_CreateConfirmation::CreateConfirmationDialog(UObject* InWorldContextObject, FText InHeaderText, FText InBodyText, const TArray<FDialogAction>& InButtonActions)
{
	// Create a new async action with the given data.
	UAsyncAction_CreateConfirmation* Action = NewObject<UAsyncAction_CreateConfirmation>();
	Action->WorldContextObject = InWorldContextObject;

	Action->DialogDefinition = NewObject<UDialogDefinition>();
	Action->DialogDefinition->HeaderText = InHeaderText;
	Action->DialogDefinition->BodyText = InBodyText;
	Action->DialogDefinition->ButtonActions = InButtonActions;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncAction_CreateConfirmation::Activate()
{
	// Attempt to find the target local player using the given world context object.
	if (WorldContextObject && !TargetLocalPlayer)
	{
		// If the given context object is a widget, use its owner.
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer();
		}
		// If the given context object is a player controller, use its local player.
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		// If the given context object has an outer world, get the world's first local player.
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer();
			}
		}
	}

	// Create and push the confirmation dialog using the data with which this task was created.
	if (TargetLocalPlayer)
	{
		if (UCrashUIMessagingSubsystem* MessagingSubsystem = TargetLocalPlayer->GetSubsystem<UCrashUIMessagingSubsystem>())
		{
			FUIMessagingResultSignature ResultCallback = FUIMessagingResultSignature::CreateUObject(this, &UAsyncAction_CreateConfirmation::HandleConfirmationResult);
			MessagingSubsystem->CreateConfirmationDialog(DialogDefinition, ResultCallback);
			return;
		}
	}

	// If the confirmation dialog couldn't be created, handle an unknown result without broadcasting anything.
	HandleConfirmationResult(EUIMessageResult::Unknown);
}

void UAsyncAction_CreateConfirmation::HandleConfirmationResult(EUIMessageResult ConfirmationResult)
{
	// Broadcast the selected response and destroy the dialog when a response is selected by the user.
	ResultDelegate.Broadcast(ConfirmationResult);

	SetReadyToDestroy();
}
