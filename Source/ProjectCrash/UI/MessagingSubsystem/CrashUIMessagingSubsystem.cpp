// Copyright Samuel Reitich. All rights reserved.


#include "UI/MessagingSubsystem/CrashUIMessagingSubsystem.h"

#include "DialogWidget.h"
#include "CrashGameplayTags.h"
#include "Player/PlayerControllers/CrashPlayerController_DEP.h"

bool UCrashUIMessagingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<ULocalPlayer>(Outer)->GetGameInstance()->IsDedicatedServerInstance())
	{
		TArray<UClass*> OutChildClasses;
		GetDerivedClasses(GetClass(), OutChildClasses, false);

		// Don't create an instance if there is an overriding implementation somewhere else.
		return OutChildClasses.Num() == 0;
	}

	return false;
}

void UCrashUIMessagingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load the dialog classes.
	ConfirmationDialogClass = ConfirmationDialogClassPath.LoadSynchronous();
	ErrorDialogClass = ErrorDialogClassPath.LoadSynchronous();
}

void UCrashUIMessagingSubsystem::CreateConfirmationDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback)
{
	APlayerController* PC = GetLocalPlayerChecked()->GetPlayerController(GetWorld());
	ACrashPlayerController_DEP* CrashPC = PC ? Cast<ACrashPlayerController_DEP>(PC) : nullptr;

	if (CrashPC)
	{
		// Create and initialize a new dialog widget with the given data.
		UCommonActivatableWidget* NewWidget = CrashPC->PushWidgetToLayer(ConfirmationDialogClass, CrashGameplayTags::TAG_UI_Layer_Dialog);
		if (UDialogWidget* WidgetAsDialog = Cast<UDialogWidget>(NewWidget))
		{
			WidgetAsDialog->SetupDialog(DialogDefinition, ResultCallback);
		}
	}
}

void UCrashUIMessagingSubsystem::CreateErrorDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback)
{
	APlayerController* PC = GetLocalPlayerChecked()->GetPlayerController(GetWorld());
	ACrashPlayerController_DEP* CrashPC = PC ? Cast<ACrashPlayerController_DEP>(PC) : nullptr;

	if (CrashPC)
	{
		// Create and initialize a new dialog widget with the given data.
		UCommonActivatableWidget* NewWidget = CrashPC->PushWidgetToLayer(ErrorDialogClass, CrashGameplayTags::TAG_UI_Layer_Dialog);
		if (UDialogWidget* WidgetAsDialog = Cast<UDialogWidget>(NewWidget))
		{
			WidgetAsDialog->SetupDialog(DialogDefinition, ResultCallback);
		}
	}
}
