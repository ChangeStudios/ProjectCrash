// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/GameModes/Game/CrashGameModeData.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/GlobalLayeredWidget.h"
#include "UI/Widgets/SlottedEntryBox.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void ACrashPlayerController::BeginPlay()
{
	const AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	const ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;
	const UCrashGameModeData* GMData = CrashGS ? CrashGS->GetGameModeData() : nullptr;
	const UUserInterfaceData* UIData = GMData ? GMData->UIData : nullptr;

	UE_LOG(LogPlayerController, VeryVerbose, TEXT("GS? %s, CrashGS? %s, GMData? %s, UIData? %s"), *CONDITIONAL_STRING(GS), *CONDITIONAL_STRING(CrashGS), *CONDITIONAL_STRING(GMData), *CONDITIONAL_STRING(UIData))

	// Create and activate the base widget. All widgets we create from here in the future will be pushed to this.
	if (UIData && UIData->GlobalLayeredWidget)
	{
		if (IsLocalPlayerController())
		{
			GlobalLayeredWidget = CreateWidget<UGlobalLayeredWidget>(this, UIData->GlobalLayeredWidget.Get());

			if (GlobalLayeredWidget)
			{
				GlobalLayeredWidget->AddToViewport();
			}
		}
	}
	else
	{
		// UE_LOG(LogPlayerController, Fatal, TEXT("ACrashPlayerController: UIData is not defined. This player controller must have a valid UIData asset and BaseWidget asset to manage the user interface."));
	}

	// If the global widget was successfully created, push the layout and slotted widgets to it.
	if (GlobalLayeredWidget && UIData)
	{
		// Push and register each layout widget.
		for (const FLayoutWidget& LayoutWidget : UIData->LayoutWidgets)
		{
			if (LayoutWidget.LayoutWidgetClass)
			{
				if (UCommonActivatableWidget* NewLayout = PushWidgetToLayer(LayoutWidget.LayoutWidgetClass.Get(), LayoutWidget.TargetLayer))
				{
					// Register the new layout widget.
					RegisteredLayoutWidgets.Add(NewLayout);

					// Register each slotted entry in the new layout widget.
					TArray<UWidget*> OutWidgets;
					NewLayout->WidgetTree->GetAllWidgets(OutWidgets);

					for (UWidget* NewWidget : OutWidgets)
					{
						if (USlottedEntryBox* Slot = Cast<USlottedEntryBox>(NewWidget))
						{
							RegisteredSlots.AddUnique(Slot);
						}
					}
				}
			}
		}

		// Add each slotted widget to the layout widgets.
		for (const FSlottedWidget& SlottedWidget : UIData->SlottedWidgets)
		{
			AddWidgetToSlot(SlottedWidget);
		}
	}

	Super::BeginPlay();
}

UCommonActivatableWidget* ACrashPlayerController::PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag LayerToPushTo)
{
	if (!GlobalLayeredWidget)
	{
		UE_LOG(LogPlayerController, Warning, TEXT("ACrashPlayerController: Could not push widget. BaseWidget has not yet been created."));
		return nullptr;
	}

	// Push the given widget to the specified layer.
	UCommonActivatableWidget* NewWidget = nullptr;

	if (LayerToPushTo.MatchesTagExact(CrashGameplayTags::TAG_UI_Layer_Game))
	{
		NewWidget = GlobalLayeredWidget->GameLayerStack->AddWidget(WidgetToPush);
	}
	else if (LayerToPushTo.MatchesTagExact(CrashGameplayTags::TAG_UI_Layer_GameMenu))
	{
		NewWidget = GlobalLayeredWidget->GameMenuStack->AddWidget(WidgetToPush);
	}
	else if (LayerToPushTo.MatchesTagExact(CrashGameplayTags::TAG_UI_Layer_Menu))
	{
		NewWidget = GlobalLayeredWidget->MenuStack->AddWidget(WidgetToPush);
	}

	return NewWidget ? NewWidget : nullptr;
}

void ACrashPlayerController::AddWidgetToSlot(const FSlottedWidget& SlottedWidgetToCreate)
{
	// Search the registered slots for one with a matching ID, and add the widget to that slot.
	for (USlottedEntryBox* SlottedWidget : RegisteredSlots)
	{
		if (SlottedWidget->SlotID.MatchesTagExact(SlottedWidgetToCreate.SlotID))
		{
			SlottedWidget->CreateEntry(SlottedWidgetToCreate.SlottedWidgetClass);
		}
	}
}
