// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

#include "GameplayTagContainer.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "UI/Data/UserInterfaceData.h"
#include "UI/Framework/GlobalLayeredWidget.h"
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
			// Register the global layered widget's layers, to and from which layer widgets will be pushed and popped.
			TArray<UWidget*> OutWidgets;
			GlobalLayeredWidget->WidgetTree->GetAllWidgets(OutWidgets);
			for (UWidget* Widget : OutWidgets)
			{
				if (UTaggedActivatableWidgetStack* WidgetAsStack = Cast<UTaggedActivatableWidgetStack>(Widget))
				{
					GlobalLayeredWidgetLayers.Add(WidgetAsStack);
				}
			}

			GlobalLayeredWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogPlayerController, Error, TEXT("ACrashPlayerController: UIData is not defined. This player controller must have a valid UIData asset and BaseWidget asset to manage the user interface."));
	}

	/* If the global widget was successfully created, push the initial layer widgets to it, and push the slotted
	 * widgets to those layer widgets. */
	if (GlobalLayeredWidget && UIData)
	{
		// Push and register each layer widget.
		for (const FLayerWidget& LayerWidget : UIData->InitialLayerWidgets)
		{
			if (LayerWidget.LayerWidgetClass)
			{
				// Push the widget.
				if (UCommonActivatableWidget* NewLayer = PushWidgetToLayer(LayerWidget.LayerWidgetClass, LayerWidget.TargetLayer))
				{
					// Register each slotted entry in the new layer widget.
					TArray<UWidget*> OutWidgets;
					NewLayer->WidgetTree->GetAllWidgets(OutWidgets);

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

		// Add each slotted widget to the layer widgets.
		for (const FSlottedWidget& SlottedWidget : UIData->SlottedWidgets)
		{
			AddWidgetToSlot(SlottedWidget);
		}
	}
}

UCommonActivatableWidget* ACrashPlayerControllerBase::PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag LayerToPushTo)
{
	UCommonActivatableWidget* NewWidget = nullptr;

	// Search for the specified layer.
	for (UTaggedActivatableWidgetStack* WidgetLayer : GlobalLayeredWidgetLayers)
	{
		if (WidgetLayer->StackID.MatchesTagExact(LayerToPushTo))
		{
			// Push the widget to the specified layer.
			NewWidget = WidgetLayer->AddWidget(WidgetToPush);

			// Register each of the new layer widget's stacks.
			TArray<UWidget*> OutWidgets;
			NewWidget->WidgetTree->GetAllWidgets(OutWidgets);
			for (UWidget* Widget : OutWidgets)
			{
				if (UTaggedActivatableWidgetStack* WidgetAsStack = Cast<UTaggedActivatableWidgetStack>(Widget))
				{
					LayerWidgetStacks.Add(WidgetAsStack);
				}
			}

			break;
		}
	}

	return NewWidget ? NewWidget : nullptr;
}

void ACrashPlayerControllerBase::PopWidgetFromLayer(FGameplayTag LayerToPopFrom)
{
	// Search for the specified layer.
	for (UTaggedActivatableWidgetStack* WidgetLayer : GlobalLayeredWidgetLayers)
	{
		if (WidgetLayer->StackID.MatchesTagExact(LayerToPopFrom))
		{
			// Retrieve the layer's top widget.
			UCommonActivatableWidget* ActiveWidget = WidgetLayer->GetActiveWidget();

			// Unregister each of the widget's stacks before popping it.
			TArray<UWidget*> OutWidgets;
			ActiveWidget->WidgetTree->GetAllWidgets(OutWidgets);
			for (UWidget* Widget : OutWidgets)
			{
				if (UTaggedActivatableWidgetStack* WidgetAsStack = Cast<UTaggedActivatableWidgetStack>(Widget))
				{
					LayerWidgetStacks.Remove(WidgetAsStack);
				}
			}

			// Pop the widget.
            WidgetLayer->RemoveWidget(*ActiveWidget);
		}
	}
}

UCommonActivatableWidget* ACrashPlayerControllerBase::PushWidgetToStack(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag StackToPushTo)
{
	UCommonActivatableWidget* NewWidget = nullptr;

	// Search for the specified stack.
	for (UTaggedActivatableWidgetStack* WidgetStack : LayerWidgetStacks)
	{
		if (WidgetStack->StackID.MatchesTagExact(StackToPushTo))
		{
			// Push the widget to the specified stack.
			NewWidget = WidgetStack->AddWidget(WidgetToPush);
		}
	}

	return NewWidget ? NewWidget : nullptr;
}

void ACrashPlayerControllerBase::PopWidgetFromStack(FGameplayTag StackToPopFrom)
{
	// Search for the specified stack.
	for (UTaggedActivatableWidgetStack* WidgetStack : LayerWidgetStacks)
	{
		if (WidgetStack->StackID.MatchesTagExact(StackToPopFrom))
		{
			// Pop the stack's top widget.
			WidgetStack->RemoveWidget(*WidgetStack->GetActiveWidget());
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
