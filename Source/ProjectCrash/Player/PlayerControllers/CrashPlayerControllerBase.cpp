// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

#include "GameplayTagContainer.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "UI/UserInterfaceData.h"
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
				if (UCommonActivatableWidget* NewLayout = PushWidgetToStack(LayoutWidget.LayoutWidgetClass, LayoutWidget.TargetLayer))
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

void ACrashPlayerControllerBase::RegisterWidgetStack(UTaggedActivatableWidgetStack* StackToRegister)
{
	if (IsValid(StackToRegister))
	{
		RegisteredWidgetStacks.AddUnique(StackToRegister);
	}
}

void ACrashPlayerControllerBase::UnregisterWidgetStack(UTaggedActivatableWidgetStack* StackToUnregister)
{
	if (RegisteredWidgetStacks.Contains(StackToUnregister))
	{
		RegisteredWidgetStacks.Remove(StackToUnregister);
	}
}

UCommonActivatableWidget* ACrashPlayerControllerBase::PushWidgetToStack(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag StackToPushTo)
{
	// Push the given widget to the specified stack.
	UCommonActivatableWidget* NewWidget = nullptr;

	for (UTaggedActivatableWidgetStack* Stack : RegisteredWidgetStacks)
	{
		if (Stack->StackID.MatchesTagExact(StackToPushTo))
		{
			NewWidget = Stack->AddWidget(WidgetToPush);
		}
	}

	return NewWidget ? NewWidget : nullptr;
}

void ACrashPlayerControllerBase::PopWidgetFromStack(FGameplayTag StackToPopFrom)
{
	// Pop the top widget from the specified stack.
	for (UTaggedActivatableWidgetStack* Stack : RegisteredWidgetStacks)
	{
		if (Stack->StackID.MatchesTagExact(StackToPopFrom))
		{
			UCommonActivatableWidget* WidgetToPop = Stack->GetActiveWidget();
			Stack->RemoveWidget(*WidgetToPop);
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
