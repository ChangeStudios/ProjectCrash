// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Data/UserInterfaceData.h"
#include "GameFramework/PlayerController.h"
#include "CrashPlayerControllerBase.generated.h"

class UCommonUserWidget;
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

	// UI.

// Initialization.
protected:

	/** Creates and caches the global widget, layout widgets, and slotted widgets defined in the given UI data. */
	UFUNCTION()
	virtual void InitializeUserInterface(const UUserInterfaceData* UIData);

// Layers.
public:

	/** Pushes the given widget to the specified layer of the global layered widget. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	UCommonActivatableWidget* PushWidgetToLayer
	(
		TSubclassOf<UCommonActivatableWidget> WidgetToPush,
		UPARAM(Meta = (Categories = "UI.Layer"))
		FGameplayTag LayerToPushTo
	);

	/** Pops the top widget from the specified layer of the global layered widget. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	void PopWidgetFromLayer
	(
		UPARAM(Meta = (Categories = "UI.Layer"))
		FGameplayTag LayerToPopFrom
	);

// Stacks.
public:

	/** Searches the registered layer widget stacks for a stack with a matching tag, and pushes this widget to the
	 * first one found. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	UCommonActivatableWidget* PushWidgetToStack
	(
		TSubclassOf<UCommonActivatableWidget> WidgetToPush,
		UPARAM(Meta = (Categories = "UI.Stack"))
		FGameplayTag StackToPushTo
	);

	/** Pops the top widget from any matching stack in the registered layer widgets. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	void PopWidgetFromStack
	(
		UPARAM(Meta = (Categories = "UI.Stack"))
		FGameplayTag StackToPopFrom
	);

protected:

	/** The global widget being rendered by this player controller, created at BeginPlay. Defines the stacks to which
	 * layer widgets are pushed. */
	UPROPERTY()
	TObjectPtr<UCommonUserWidget> GlobalLayeredWidget;

	/** Widget layers (e.g. "Game," "Menu," etc.) inside the current GlobalLayeredWidget. New layer widgets are pushed
	 * to these layers using matching tags. These layer widgets aren't explicitly cached, since we can access them as
	 * children of the layers in this array. */
	UPROPERTY()
	TArray<UTaggedActivatableWidgetStack*> GlobalLayeredWidgetLayers;

	/**
	 * Widget stacks inside registered layer widgets. When a layer widget is pushed to a global layer, each widget
	 * stack in its tree is registered here. New widgets can be pushed to and popped from these stacks using matching
	 * tags.
	 *
	 * Note: If we wanted to allow widgets pushed to widget stacks to have nested widget stacks, we could just add
	 * their stacks to this array when we create them.
	 */
	UPROPERTY()
	TArray<UTaggedActivatableWidgetStack*> LayerWidgetStacks;

// Slots.
public:

	/** Attempts to find a registered slot with a matching slot ID and adds the given widget to that slot. */
	void AddWidgetToSlot(const FSlottedWidget& SlottedWidgetToCreate);

protected:

	/** Layout widgets that have been created by this player controller. These define the layout of active widget
	 * slots. */
	UPROPERTY()
	TArray<UCommonActivatableWidget*> RegisteredLayoutWidgets;

	/** Slotted widgets within the registered layout widgets. New slotted widgets will be created within any slots here
     * that match their slot ID. */
    UPROPERTY()
    TArray<USlottedEntryBox*> RegisteredSlots;
};
