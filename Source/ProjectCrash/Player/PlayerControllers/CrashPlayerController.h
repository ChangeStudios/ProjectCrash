// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/UserInterfaceData.h"
#include "CrashPlayerController.generated.h"

class USlottedEntryBox;
class UGlobalLayeredWidget;

/**
 * The base player controller used by this project. Implements logic for client-only events, such as creating and
 * manipulating the user interface.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerController : public APlayerController
{  
	GENERATED_BODY()

	// Initialization.

public:

	/** Creates and caches the default widget defined in the UI data. */
	virtual void BeginPlay() override;



	// UI.

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