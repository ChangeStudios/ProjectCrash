// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UserInterfaceData.generated.h"

class UGlobalLayeredWidget;
class UCommonActivatableWidget;


/**
 * A widget that is pushed directly to a widget layer ("Game," "UI," etc.), inside of which slotted widgets and layer
 * widget stacks will be created.
 */
USTRUCT()
struct FLayerWidget
{
	GENERATED_BODY()

	/** The layer widget to create. */
	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UCommonActivatableWidget> LayerWidgetClass;

	/** The layer to which to push the layer widget. */
	UPROPERTY(EditAnywhere, Category = UserInterface, Meta = (Categories = "UI.Layer"))
	FGameplayTag TargetLayer;
};


/**
 * A widget that will be created in a specified slot within any existing layer widget or layer widget stack with that
 * slot.
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

	/** The global widget that will be created and pushed when the game starts. All other widgets will be pushed to the
	 * layers within in this widget. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic")
	TSubclassOf<UGlobalLayeredWidget> GlobalLayeredWidget;

	/** Widgets that will be created and pushed directly to corresponding widget layers within the global layered
	 * widget. These widgets will be searched when creating slotted widgets. Additional layer widgets can be pushed and
	 * popped during runtime. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic", Meta = (TitleProperty = "{TargetLayer} -> {LayerWidgetClass}"))
	TArray<FLayerWidget> InitialLayerWidgets;

	/** Widgets that will be created and placed inside any slot with a matching tag within any initial layer widget
	 * within the global layered widget. These widgets should NOT change during runtime. For runtime-manipulative
	 * widgets, use layer widget stacks. */
	UPROPERTY(EditDefaultsOnly, Category = "Generic", Meta = (TitleProperty = "{SlotID} -> {SlottedWidgetClass}"))
	TArray<FSlottedWidget> SlottedWidgets;
};
