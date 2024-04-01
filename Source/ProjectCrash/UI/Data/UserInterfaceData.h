// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UserInterfaceData.generated.h"

class UGlobalLayeredWidget;
class UCommonActivatableWidget;


/**
 * A widget that defines the layouts for slotted widgets, inside of which slotted widgets will be created.
 */
USTRUCT()
struct FLayoutWidget
{
	GENERATED_BODY()

	/** The layout widget to create. */
	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UCommonActivatableWidget> LayoutWidgetClass;

	/** The layer to which to push the layer. */
	UPROPERTY(EditAnywhere, Category = UserInterface, Meta = (Categories = "UI.Layer"))
	FGameplayTag TargetLayer;
};


/**
 * A widget that will be created in a specified slot within any existing layout widget with that slot.
 */
USTRUCT()
struct FSlottedWidget
{
	GENERATED_BODY()

	/** The slotted widget to create. */
	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UCommonActivatableWidget> SlottedWidgetClass;

	/** The slot where this widget should be placed. */
	UPROPERTY(EditAnywhere, Category = UserInterface, Meta = (Categories = "UI.Slot"))
	FGameplayTag SlotID;
};


/**
 * A global collection of data defining the user interface.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCRASH_API UUserInterfaceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** The global widget that will be created and pushed when the game starts. All other widgets will be pushed to
	 * TaggedActivatableWidgetStacks in this widget. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic")
	TSubclassOf<UCommonActivatableWidget> GlobalLayeredWidget;

	/** Widgets that will be created and registered as "layout widgets." When modular widgets are added, each layout
	 * widget's slots will be checked for a tag matching the modular widget, and the widget will be created in that
	 * slot. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic", Meta = (TitleProperty = "{TargetLayer} -> {LayoutWidgetClass}"))
	TArray<FLayoutWidget> LayoutWidgets;

	/** Widgets that will be created and placed inside any corresponding slot defined in any existing layout
	 * widgets. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic", Meta = (TitleProperty = "{SlotID} -> {SlottedWidgetClass}"))
	TArray<FSlottedWidget> SlottedWidgets;
};
