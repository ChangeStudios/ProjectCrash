// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/UserInterfaceData.h"
#include "GameFramework/PlayerController.h"
#include "CrashPlayerControllerBase.generated.h"

class UTaggedActivatableWidgetStack;
class UCommonActivatableWidget;
class UGlobalLayeredWidget;
class USlottedEntryBox;

/**
 * Base player controller for this project. Provides utilities for managing the user interface via the Common UI plugin.
 */
UCLASS(Abstract)
class PROJECTCRASH_API ACrashPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

	// UI initialization.

protected:

	/** Creates and caches the global widget, layout widgets, and slotted widgets defined in the given UI data. */
	UFUNCTION()
	virtual void InitializeUserInterface(const UUserInterfaceData* UIData);



	/* Widget stacks. Global stacks of widgets that can have child widgets dynamically pushed to and popped from them
	 * from anywhere. */

public:

	/** Registers the given stack for this player, allowing widgets to be pushed to and popped from it globally. */
	void RegisterWidgetStack(UTaggedActivatableWidgetStack* StackToRegister);

	/** Unregisters the given stack for this player. */
	void UnregisterWidgetStack(UTaggedActivatableWidgetStack* StackToUnregister);

	/** Pushes the given widget to the specified stack, if one is registered. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Widget Stacks")
	UCommonActivatableWidget* PushWidgetToStack(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag StackToPushTo);

	/** Pops the top widget from the specific stack. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Widget Stacks")
	void PopWidgetFromStack(FGameplayTag StackToPopFrom);

protected:

	/** The global widget being rendered by this player controller, created at BeginPlay. Defines the persistent widget
	 * stacks that will be created by default. */
	UPROPERTY()
	UCommonActivatableWidget* GlobalLayeredWidget;

	/** Activatable widget stacks currently registered with this player. When a new widget is requested to pushed via
	 * tag, it will be pushed to the registered first stack with the matching tag. */
	UPROPERTY()
	TArray<UTaggedActivatableWidgetStack*> RegisteredWidgetStacks;



	// Widget slots. Abstract entry points inside of widgets that can have any kind of widget inserted in their place.

public:

	/** Attempts to find a registered slot with a matching slot ID and adds the given widget to that slot. */
	void AddWidgetToSlot(const FSlottedWidget& SlottedWidgetToCreate);

protected:

	/** Layout widgets that have been created by this player controller. These define optional slots for inserting
	 * modular widgets. These slots are added to RegisteredSlots when the layout widget is registered. */
	UPROPERTY()
	TArray<UCommonActivatableWidget*> RegisteredLayoutWidgets;

	/** Slotted widgets within the registered layout widgets. New slotted widgets will be created within any slots here
	 * that match their slot ID. */
	UPROPERTY()
	TArray<USlottedEntryBox*> RegisteredSlots;
};
