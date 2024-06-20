// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddInputActionMapping.generated.h"

struct FComponentRequestHandle;
class UCrashInputActionMapping;

/**
 * Adds input action mappings to the local player's input system. Only ability action mappings can be added (i.e. not
 * native actions). Intended to add game mode-specific ability actions.
 *
 * Any pawns that want to receive the action mappings must have a subclass of InputHandlerComponentBase, which handles
 * dynamically adding and removing action mappings.
 */
UCLASS(MinimalAPI, DisplayName = "Add Input Action Mapping")
class UGameFeatureAction_AddInputActionMapping : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

	// Game feature action.

public:

	/** Creates a new context data instance for the given context. Resets this action's context data if it already has
	 * one. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Resets this action's context data for the given context. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	/** Action mappings that will be added to the local player when this action is activated. */
	UPROPERTY(EditAnywhere, Category = "Input", Meta = (AssetBundles = "Client,Server"))
	TArray<TSoftObjectPtr<const UCrashInputActionMapping>> InputActionMappings;

	/** Extension request name for when this action should bind its action mappings. Must be added by the player when
	 * they are ready to bind input. */
	static const FName NAME_BindInputsNow;

private:

	/** Starts listening for the extension request that triggers this action. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;



	// Internals.

// Data.
private:

	/**
	 * Data mapped to a specific context to cache this action's data in that context.
	 */
	struct FPerContextData
	{
		// Delegates to the extension request to bind this action's input.
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// The pawns in the context to whom we've added this action's mappings.
		TArray<TWeakObjectPtr<APawn>> PawnsAddedTo;
	};

	/** Context-specific data for each context in which this game feature action is active. */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

// Utils.
private:

	/** Resets the given context data. */
	void Reset(FPerContextData& ActiveData);

	/** Adds or removes this action's mappings when its associated extension request is added or removed by the
	 * player. */
	void HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/** Actually adds this action's mappings to the given pawn. */
	void AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData);

	/** Removes this action's mappings to the given pawn. */
	void RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData);



	// Validation.

public:

#if WITH_EDITOR
	/** Validates this action is not using any null action mappings and that no action mappings are trying to bind
	 * native actions. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
