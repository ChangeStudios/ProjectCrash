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
	UCommonActivatableWidget* PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag LayerToPushTo);

	/** Pushes the uppermost widget from the specified layer of the global layered widget. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	void PopWidgetFromLayer(FGameplayTag LayerToPopFrom);

protected:

	/** The global widget being rendered by this player controller, created at BeginPlay. Defines the layers to which
	 * new widgets can be pushed and popped. */
	UPROPERTY()
	TObjectPtr<UCommonActivatableWidget> GlobalLayeredWidget;

	/** Widget layers inside the current GlobalLayeredWidgets. New widgets are pushed to these layers using matching
	 * tags. */
	UPROPERTY()
	TArray<UTaggedActivatableWidgetStack*> RegisteredWidgetLayers;

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
