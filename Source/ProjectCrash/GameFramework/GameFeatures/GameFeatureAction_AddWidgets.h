// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "UIExtensionSystem.h"
#include "GameFeatureAction_AddWidgets.generated.h"

class UCommonActivatableWidget;
struct FComponentRequestHandle;

/**
 * An entry in the AddWidgets game feature action defining a HUD layout to add to players.
 */
USTRUCT()
struct FGameFeatureHUDLayoutEntry
{
	GENERATED_BODY()

	/** The widget class to spawn and register as a HUD layout. This should contain tagged extension points into which
	 * modular widgets can be inserted. */
	UPROPERTY(EditAnywhere, Category = "HUD Layout", Meta = (AssetBundles = "Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	/** The layer into which the layout widget will be inserted. */
	UPROPERTY(EditAnywhere, Category = "HUD Layout", Meta = (Categories = "UI.Layer"))
	FGameplayTag TargetLayer;
};



/**
 * An entry in the AddWidgets game feature action defining a widget to insert into an extension point in players' HUDs.
 */
USTRUCT()
struct FGameFeatureWidgetEntry
{
	GENERATED_BODY()

	/** The widget class to spawn. */
	UPROPERTY(EditAnywhere, Category = "Widget", Meta = (AssetBundles = "Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** The slot into which this widget will be inserted on the player's HUD. The player needs a HUD layout containing
	 * an extension point with a matching slot ID for this widget to be successfully added to their UI. */
	UPROPERTY(EditAnywhere, Category = "Widget", Meta = (Categories = "UI.Slot"))
	FGameplayTag TargetSlot;
};



/**
 * Adds HUD layouts to all players. Modularly adds widgets to "slots" in players' HUD layouts.
 */
UCLASS(MinimalAPI, DisplayName = "Add Widgets")
class UGameFeatureAction_AddWidgets : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

	// Game feature action.

public:

	/** Resets the context data for the given context. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

private:

	/** Starts listening for extension requests from HUD actors to add or remove this action's layouts and widgets. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;



	// Action data.

private:

	/** List of HUD layouts which will be added to and registered with each player's user interface. HUD layouts serve
	 * as a (usually) empty canvas, comprised of "extension points": empty widgets, identified by a gameplay tag, into
	 * which modular widgets can be inserted. */
	UPROPERTY(EditAnywhere, Category = "UI", DisplayName = "Layouts to Add", Meta = (TitleProperty = "{TargetLayer} -> {LayoutClass}"))
	TArray<FGameFeatureHUDLayoutEntry> LayoutList;

	/** List of widgets which will be added to each player's user interface. Each widget will be inserted into the
	 * first extension point with a matching slot ID in any of the player's active HUD layouts. */
	UPROPERTY(EditAnywhere, Category = "UI", DisplayName = "Widgets to Add", Meta = (TitleProperty = "{TargetSlot} -> {WidgetClass}"))
	TArray<FGameFeatureWidgetEntry> WidgetList;



	// Internals.

// Data.
private:

	/**
	 * Handle for the HUD layouts and widgets added by this action to an actor with an active extension.
	 */
	struct FExtendedActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		TArray<FUIExtensionHandle> WidgetsAdded;
	};

	/**
	 * Data for a specific context, used to cache this action's data in that context.
	 */
	struct FPerContextData
	{
		// Delegates to the extension requests to add or remove this action's layouts and widgets.
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// Actors with an active extension adding this action's layouts and widgets to their UI. Actors should be HUDs.
		TMap<FObjectKey, FExtendedActorData> ActiveExtensions;
	};

	/** Context-specific data for each context in which this game feature action is active. */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** Resets the given context data. */
	void Reset(FPerContextData& ActiveData);

// Adding widgets.
private:

	/** Adds or removes this action's layouts and widgets to/from the given actor when the corresponding extension
	 * request is received. Actor should be a HUD. */
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	void AddWidgets(AActor* Actor, FPerContextData& ActiveData);

	void RemoveWidgets(AActor* Actor, FPerContextData& ActiveData);



	// Asset management.

public:

#if WITH_EDITORONLY_DATA
	/** Adds this action's layouts and widgets to the client asset bundle, so they're always loaded for clients. */
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif // WITH_EDITORONLY_DATA



	// Validation.

public:

#if WITH_EDITOR
	/** Validates the classes and tags used in this action. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
};
