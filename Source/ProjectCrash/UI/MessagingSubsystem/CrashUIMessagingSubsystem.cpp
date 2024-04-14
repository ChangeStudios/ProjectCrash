// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/CrashUIMessagingSubsystem.h"

#include "DialogWidget.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

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

	ConfirmationDialogClass = ConfirmationDialogClassPath.LoadSynchronous();
	ErrorDialogClass = ErrorDialogClassPath.LoadSynchronous();
}

void UCrashUIMessagingSubsystem::CreateConfirmationDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback)
{
	APlayerController* PC = GetLocalPlayerChecked()->GetPlayerController(GetWorld());
	ACrashPlayerControllerBase* CrashPC = PC ? Cast<ACrashPlayerControllerBase>(PC) : nullptr;

	if (CrashPC)
	{
		// Create and initialize a new dialog widget with the given data.
		UE_LOG(LogTemp, Error, TEXT("A"));
		UCommonActivatableWidget* NewWidget = CrashPC->PushWidgetToLayer(ConfirmationDialogClass, CrashGameplayTags::TAG_UI_Layer_Dialog);
		if (UDialogWidget* WidgetAsDialog = Cast<UDialogWidget>(NewWidget))
		{
			UE_LOG(LogTemp, Error, TEXT("A"));

			WidgetAsDialog->SetupDialog(DialogDefinition, ResultCallback);
		}
	}
}

void UCrashUIMessagingSubsystem::CreateErrorDialog(UDialogDefinition* DialogDefinition, FUIMessagingResultSignature ResultCallback)
{
	APlayerController* PC = GetLocalPlayerChecked()->GetPlayerController(GetWorld());
	ACrashPlayerControllerBase* CrashPC = PC ? Cast<ACrashPlayerControllerBase>(PC) : nullptr;

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
