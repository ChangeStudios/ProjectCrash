// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

#include "GameplayTagContainer.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "UI/Data/UserInterfaceData.h"
#include "UI/Widgets/GlobalLayeredWidget.h"
#include "UI/Widgets/Utils/SlottedEntryBox.h"
#include "UI/Widgets/Utils/TaggedActivatableWidgetStack.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void ACrashPlayerControllerBase::InitializeUserInterface(const UUserInterfaceData* UIData)
{
	// Only handle UI on the local machine.
	if (!IsLocalPlayerController())
	{
		return;
	}

	/* Create and activate the base widget and cache its layers. All widgets we create from here in the future will be
	 * pushed to this. */
	if (UIData && UIData->GlobalLayeredWidget)
	{
		GlobalLayeredWidget = CreateWidget<UCommonActivatableWidget>(this, UIData->GlobalLayeredWidget);

		if (GlobalLayeredWidget)
		{
			// Register the global layered widget's stacks to which widgets will be pushed and popped.
			TArray<UWidget*> OutWidgets;
			GlobalLayeredWidget->WidgetTree->GetAllWidgets(OutWidgets);
			for (UWidget* Widget : OutWidgets)
			{
				if (UTaggedActivatableWidgetStack* WidgetAsStack = Cast<UTaggedActivatableWidgetStack>(Widget))
				{
					RegisteredWidgetLayers.Add(WidgetAsStack);
				}
			}

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

UCommonActivatableWidget* ACrashPlayerControllerBase::PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag LayerToPushTo)
{
	UCommonActivatableWidget* NewWidget = nullptr;

	// Push the given widget to the specified layer.
	for (UTaggedActivatableWidgetStack* WidgetLayer : RegisteredWidgetLayers)
	{
		if (WidgetLayer->StackID.MatchesTagExact(LayerToPushTo))
		{
			NewWidget = WidgetLayer->AddWidget(WidgetToPush);
		}
	}

	return NewWidget ? NewWidget : nullptr;
}

void ACrashPlayerControllerBase::PopWidgetFromLayer(FGameplayTag LayerToPopFrom)
{
	// Pop the top widget from the specified layer.
	for (UTaggedActivatableWidgetStack* WidgetLayer : RegisteredWidgetLayers)
	{
		if (WidgetLayer->StackID.MatchesTagExact(LayerToPopFrom))
		{
			UCommonActivatableWidget* ActiveWidget = WidgetLayer->GetActiveWidget();
            WidgetLayer->RemoveWidget(*ActiveWidget);
		}
	}
}

void ACrashPlayerControllerBase::AddWidgetToSlot(const FSlottedWidget& SlottedWidgetToCreate)
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
