// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddGameModeProperties.generated.h"

class UGameModePropertySubsystem;

/**
 * An entry in the AddGameModeProperties game feature action. Defines a KVP of a property tag and its value.
 */
USTRUCT()
struct FGameFeaturePropertiesEntry
{
	GENERATED_BODY()

	/** The game mode property to register. */
	UPROPERTY(EditAnywhere, Category = "Game Mode Properties", Meta = (Categories = "GameMode.Property"))
	FGameplayTag Property;

	/** The value of the game mode property. */
	UPROPERTY(EditAnywhere, Category = "Game Mode Properties")
	float Value;
};



/**
 * Registers a collection of game mode properties with the game mode property subsystem.
 *
 * A game mode property is an identifying gameplay tag mapped to a number, which is globally readable via the game mode
 * property subsystem. These are useful for game mode-specific variables that need to be referenced by multiple systems:
 * respawn time, game phase duration, etc.
 *
 * @see UGameModePropertySubsystem.
 */
UCLASS(MinimalAPI, DisplayName = "Add Game Mode Properties")
class UGameFeatureAction_AddGameModeProperties : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

	// Game feature action.

public:

	/** Creates a new structure (context data) for tracking the game mode properties that will be added to game mode
	 * property subsystem in the given context. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Removes this action's game mode properties from the game mode property subsystem in the given context. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

private:

	/** Registers this action's game mode properties with the world's game mode property subsystem. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;



	// Action data.

public:

	/** The game mode properties that will be registered. These will be globally readable via the game mode property
	 * subsystem. */
	// TODO: Should we just make this a TMap?
	UPROPERTY(EditAnywhere, Category = "Game Mode Properties", DisplayName = "Properties to Register", Meta = (TitleProperty = "{Property}: {Value}", ShowOnlyInnerProperties))
	TArray<FGameFeaturePropertiesEntry> GameModePropertiesList;



	// Internals.

private:

	/**
	 * Data for a specific context, used to cache the game mode properties registered to the subsystem in that context.
	 */
	struct FPerContextData
	{
		// The context's game mode property subsystem.
		TWeakObjectPtr<UGameModePropertySubsystem> GameModePropertySubsystem;
		// Game mode properties that this action has registered with the context's game mode property subsystem.
		TArray<FGameplayTag> RegisteredProperties;
	};

	/** Context-specific data for each context in which this game feature action is active. */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** Resets the given context data. */
	void Reset(FPerContextData& ActiveData);



	// Validation.

public:

#if WITH_EDITOR
	/** Prevents duplicate game mode property entries. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
};
