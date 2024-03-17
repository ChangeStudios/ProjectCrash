// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/UserInterfaceData.h"
#include "GameFramework/PlayerController.h"
#include "CrashPlayerControllerBase.generated.h"

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

// Management.
public:

	/** Pushes the given widget to the specific layer. */
	UFUNCTION(BlueprintCallable, Category = "UserInterface|Player Controller")
	UCommonActivatableWidget* PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, FGameplayTag LayerToPushTo);

	/** Attempts to find a registered slot with a matching slot ID and adds the given widget to that slot. */
	void AddWidgetToSlot(const FSlottedWidget& SlottedWidgetToCreate);

protected:

	/** The global widget being rendered by this player controller, created at BeginPlay. New widgets are pushed
	 * here. */
	UPROPERTY()
	UGlobalLayeredWidget* GlobalLayeredWidget;

	/** Layout widgets that have been created by this player controller. */
	UPROPERTY()
	TArray<UCommonActivatableWidget*> RegisteredLayoutWidgets;

	/** Slotted widgets within the registered layout widgets. New slotted widgets will be created within any slots here
	 * that match their slot ID. */
	UPROPERTY()
	TArray<USlottedEntryBox*> RegisteredSlots;
};
