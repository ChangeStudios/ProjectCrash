// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/GameModes/Game/CrashGameModeData.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/GlobalLayeredWidget.h"
#include "UI/Widgets/Utils/SlottedEntryBox.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void ACrashPlayerController::InitializeUserInterface(const UCrashGameModeData* GameModeData)
{
	// Only handle UI on the local machine.
	if (!IsLocalPlayerController())
	{
		return;
	}

	const UUserInterfaceData* UIData = GameModeData ? GameModeData->UIData : nullptr;

	// Create and activate the base widget. All widgets we create from here in the future will be pushed to this.
	if (UIData && UIData->GlobalLayeredWidget)
	{
		GlobalLayeredWidget = CreateWidget<UGlobalLayeredWidget>(this, UIData->GlobalLayeredWidget);

		if (GlobalLayeredWidget)
		{
			GlobalLayeredWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogPlayerController, Error, TEXT("ACrashPlayerController: UIData is not defined. This player controller must have a valid UIData asset and BaseWidget asset to manage the user interface."));
	}

	// If the global widget was successfully created, push the layout and slotted widgets to it.
	if (GlobalLayeredWidget && UIData)
	{
		// Push and register each layout widget.
		for (const FLayoutWidget& LayoutWidget : UIData->LayoutWidgets)
		{
			if (LayoutWidget.LayoutWidgetClass)
			{
				if (UCommonActivatableWidget* NewLayout = PushWidgetToLayer(LayoutWidget.LayoutWidgetClass, LayoutWidget.TargetLayer))
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
}

void ACrashPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Broadcast that this controller's player state changed.
	PlayerStateChangedDelegate.Broadcast();
}

void ACrashPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;

	/* If the game mode data is already valid (i.e. this is a listen server's player), immediately initialize this
	 * player's UI. */
	if (CrashGS->GetGameModeData())
	{
		InitializeUserInterface(CrashGS->GetGameModeData());
	}
	/* If the game mode data has not been replicated yet, assign InitializeUserInterface as a callback to when it
	 * becomes available. */
	else
	{
		CrashGS->OnGameModeDataReplicated.AddDynamic(this, &ACrashPlayerController::InitializeUserInterface);
	}
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
