// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "AbilitySystem/Components/AbilitySystemExtensionComponent.h"
#include "UI/Widgets/LayeredBaseWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void ACrashPlayerController::BeginPlay()
{
	// Create and activate the base widget. All widgets we create from here in the future will be pushed to this.
	if (UIData && UIData->BaseWidget)
	{
		if (IsLocalPlayerController())
		{
			BaseWidget = CreateWidget<ULayeredBaseWidget>(this, UIData->BaseWidget);

			if (BaseWidget)
			{
				BaseWidget->AddToViewport();
			}
		}
	}
	else
	{
		UE_LOG(LogPlayerController, Fatal, TEXT("ACrashPlayerController: UIData is not defined. This player controller must have a valid UIData asset and BaseWidget asset to manage the user interface."));
	}

	Super::BeginPlay();
}

UCommonActivatableWidget* ACrashPlayerController::PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, EUserInterfaceLayer LayerToPushTo)
{
	if (!BaseWidget)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("ACrashPlayerController: Could not push widget. BaseWidget has not yet been created."));
		return nullptr;
	}

	// Push the given widget to the specified layer.
	UCommonActivatableWidget* NewWidget = nullptr;
	switch (LayerToPushTo)
	{
		case Game:
		{
			NewWidget = BaseWidget->GameLayerStack->AddWidget(WidgetToPush);
			break;
		}

		case GameMenu:
		{
			NewWidget = BaseWidget->GameMenuStack->AddWidget(WidgetToPush);
			break;
		}

		case Menu:
		{
			NewWidget = BaseWidget->MenuStack->AddWidget(WidgetToPush);
			break;
		}
	}

	return NewWidget ? NewWidget : nullptr;
}
