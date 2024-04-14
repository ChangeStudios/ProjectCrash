// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/Actions/AsyncAction_CreateConfirmation.h"

#include "UI/MessagingSubsystem/DialogWidget.h"

UAsyncAction_CreateConfirmation* UAsyncAction_CreateConfirmation::CreateConfirmationDialog(UObject* InWorldContextObject, FText InHeaderText, FText InBodyText, const TArray<FDialogAction>& InButtonActions)
{
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
	if (WorldContextObject && !TargetLocalPlayer)
	{
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer();
		}
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer();
			}
		}
	}

	if (TargetLocalPlayer)
	{
		if (UCrashUIMessagingSubsystem* MessagingSubsystem = TargetLocalPlayer->GetSubsystem<UCrashUIMessagingSubsystem>())
		{
			FUIMessagingResultSignature ResultCallback = FUIMessagingResultSignature::CreateUObject(this, &UAsyncAction_CreateConfirmation::HandleConfirmationResult);
			MessagingSubsystem->CreateConfirmationDialog(DialogDefinition, ResultCallback);
			return;
		}
	}

	HandleConfirmationResult(EUIMessageResult::Unknown);
}

void UAsyncAction_CreateConfirmation::HandleConfirmationResult(EUIMessageResult ConfirmationResult)
{
	ResultDelegate.Broadcast(ConfirmationResult);

	SetReadyToDestroy();
}
