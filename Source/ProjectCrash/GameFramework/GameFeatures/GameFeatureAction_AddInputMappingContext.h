// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddInputMappingContext.generated.h"

struct FComponentRequestHandle;
struct FPrioritizedInputMappingContext;

/**
 * Adds input mapping contexts to the local player's input system. Intended to add contexts for game mode-specific
 * ability actions.
 *
 * Any players that want to add the mapping contexts must add the NAME_AddContextsNow extension request when
 * they are ready to bind input (usually right after they initialize their input component). This can be handled
 * automatically by using a subclass of the InputHandlerComponentBase component.
 */
UCLASS(MinimalAPI, DisplayName = "Add Input Mapping Context")
class UGameFeatureAction_AddInputMappingContext : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

	// Game feature action.

public:

	/** Binds a delegate to register this action's mapping contexts when a new game instance starts. Registers the
	 * mapping contexts with the input registry subsystem for any game instances that have already started. */
	virtual void OnGameFeatureRegistering() override;

	/** Creates a new context data instance for the given context. Resets the context's context data if it already has
	 * one. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Resets the context data for the given context. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	/** Unregisters this action's mapping contexts. */
	virtual void OnGameFeatureUnregistering() override;

	/** Mapping contexts that will be registered and added when this action is activated. */
	// TODO: Make soft
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FPrioritizedInputMappingContext> InputMappingContexts;

	/** Extension request name for when this action should add its mappings contexts. Must be added by the player
	 * when they are ready to bind input. */
	static const FName NAME_AddContextsNow;

private:

	/** Starts listening for the extension request that adds and removes this action's mapping contexts to and from the
	 * local player: NAME_AddContextsNow. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;



	// Internals.

// Data.
private:

	/**
	 * Data mapped to a specific context to cache this action's data in that context.
	 */
	struct FPerContextData
	{
		// Delegates to the extension request to add this action's mapping contexts.
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// The controllers in the context to whom we've added this action's mapping contexts.
		// TODO: Remove. We never actually add anything to this.
		TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;
	};

	/** Context-specific data for each context in which this game feature action is active. */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** Resets the given context data. */
	void Reset(FPerContextData& ActiveData);

// Registering contexts.
private:

	/** Delegate to register this action's mapping contexts when a new game instance starts. */
	FDelegateHandle RegisterInputContextMappingsForGameInstanceHandle;

	/** Registers this action's mapping contexts for each local player in the given game instance. Also binds delegates
	 * to register/unregister the contexts for local players when they're added to or removed from the game. */
	void RegisterInputMappingContextsForGameInstance(UGameInstance* GameInstance);

	/** Registers this action's input mapping contexts for the local player. */
	void RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	/** Unregisters this action's input mapping contexts for all local players in the given game instance. */
	void UnregisterInputMappingContextsForGameInstance(UGameInstance* GameInstance);

	/** Unregisters this action's input mapping contexts from the local player. */
	void UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

// Adding contexts.
private:

	/** Adds or removes this action's mapping contexts from a local player when that player adds or removes their
	 * extension request. */
	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/** Adds this action's mapping contexts to the given local player. */
	void AddMappingContextForPlayer(UPlayer* Player, FPerContextData& ActiveData);

	/** Removes this action's mapping contexts from the given local player. */
	void RemoveMappingContext(UPlayer* Player, FPerContextData& ActiveData);



	// Validation.

public:

#if WITH_EDITOR
	/** Validates that this action is not using any null mapping contexts. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
