// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/UserInterfaceData.h"
#include "CrashPlayerController.generated.h"

class ULayeredBaseWidget;

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
	UCommonActivatableWidget* PushWidgetToLayer(TSubclassOf<UCommonActivatableWidget> WidgetToPush, EUserInterfaceLayer LayerToPushTo);

protected:

	/** The base widget being rendered by this player controller, created at BeginPlay. New widgets are pushed here. */
	UPROPERTY()
	ULayeredBaseWidget* BaseWidget;

	/** User interface data used by this player controller. The player controller is responsible for handling UI. */
	UPROPERTY(EditDefaultsOnly, Category = UserInterface)
	TObjectPtr<UUserInterfaceData> UIData;
};