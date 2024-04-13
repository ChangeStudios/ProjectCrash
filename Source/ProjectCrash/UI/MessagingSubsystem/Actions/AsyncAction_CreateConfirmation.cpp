// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/Actions/AsyncAction_CreateConfirmation.h"

UAsyncAction_CreateConfirmation* UAsyncAction_CreateConfirmation::CreateConfirmation(UObject* InWorldContextObject, UDialogDefinition* DialogDefinition)
{
	return nullptr;
}

void UAsyncAction_CreateConfirmation::Activate()
{
	Super::Activate();
}

void UAsyncAction_CreateConfirmation::HandleConfirmationResult(EUIMessageResult ConfirmationResult)
{
}
