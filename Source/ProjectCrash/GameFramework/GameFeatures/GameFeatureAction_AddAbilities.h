// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameFramework/GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "AbilitySystem/Abilities/CrashAbilitySet.h"
#include "GameFeatureAction_AddAbilities.generated.h"

class UAttributeSet;
class UCrashAbilitySet;
class UGameplayAbility;
struct FComponentRequestHandle;
struct FGameplayAbilitySpecHandle;

/**
 * An attribute set class and optional data used to initialize it.
 */
USTRUCT(BlueprintType)
struct FGameFeatureAbilitiesEntry_AttributeSet
{
	GENERATED_BODY()

	/** Attribute set to grant. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AssetBundles = "Client,Server"))
	TSoftClassPtr<UAttributeSet> AttributeSetClass;

	/** Data table with which to initialize the attribute set, if any (can be left unset). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AssetBundles = "Client,Server"))
	TSoftObjectPtr<UDataTable> InitializationData;
};



/**
 * An entry in the AddAbilities game feature action. Defines abilities, ability sets, attribute sets, and the class of
 * actor to which they will be granted.
 */
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()

	/** The class of actor to which this game feature's abilities and attributes will be granted. */
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AActor> ActorClass;

	/** Abilities to grant to actors of the specified class. */
	UPROPERTY(EditAnywhere, Category="Abilities", DisplayName = "Abilities", Meta = (AssetBundles = "Client,Server"))
	TArray<TSoftClassPtr<UGameplayAbility>> GrantedAbilities;

	/** Ability sets to grant to actors of the specified class. Target ASC must be of type CrashAbilitySystemComponent
	 * to receive ability sets. */
	UPROPERTY(EditAnywhere, Category="Abilities", DisplayName = "Ability Sets", Meta = (AssetBundles = "Client,Server"))
	TArray<TSoftObjectPtr<const UCrashAbilitySet>> GrantedAbilitySets;

	/** Attribute sets to grant to actors of the specified class. */
	UPROPERTY(EditAnywhere, Category="Attributes", DisplayName = "Attribute Sets")
	TArray<FGameFeatureAbilitiesEntry_AttributeSet> GrantedAttributeSets;
};



/**
 * Adds abilities and attribute sets to all actors of a specified class.
 */
UCLASS(MinimalAPI, DisplayName = "Add Abilities/Attributes")
class UGameFeatureAction_AddAbilities : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

	// Game feature action.

public:

	/** Creates a new context data instance for the given context. Resets the context's context data if it already has
	 * one. */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;

	/** Resets the context data for the given context. */
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	/** Extension request name for when this action should grant its abilities. Must be added by the receiving actor
	 * when they are ready to be granted abilities. */
	static const FName NAME_AbilitiesReady;

private:

	/** Starts listening for the extension request that triggers this action: NAME_AbilitiesReady. */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;



	// Action data.

public:

	/** Abilities, ability sets, and attribute sets that will be granted to actors of a specified class. */
	UPROPERTY(EditAnywhere, Category="Abilities/Attributes", DisplayName = "Granted Abilities/Attributes List", Meta = (TitleProperty = "ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureAbilitiesEntry> AbilitiesList;



	// Internals.

// Data.
private:

	/**
	 * Handle for the abilities and attribute sets granted by this action to an actor with an active extension.
	 */
	struct FExtendedActorData
	{
		TArray<FGameplayAbilitySpecHandle> Abilities;
		TArray<UAttributeSet*> AttributeSets;
		TArray<FCrashAbilitySet_GrantedHandles> AbilitySetHandles;
	};

	/**
	 * Data for a specific context, used to cache this action's data in that context.
	 */
	struct FPerContextData
	{
		// Delegates to the extension requests to grant or remove this action's abilities and attribute sets.
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		// Actors with an active extension granting this action's abilities and attribute sets to them.
		TMap<AActor*, FExtendedActorData> ActiveExtensions;
	};

	/** Context-specific data for each context in which this game feature action is active. */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	/** Resets the given context data. */
	void Reset(FPerContextData& ActiveData);

// Granting abilities.
private:

	/** Adds or removes this action's abilities and attribute sets to/from an actor when the corresponding extension
	 * request is received. */
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);

	/** Grants the abilities and attribute sets in the specified entry to the given actor. */
	void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);

	/** Removes the abilities and attribute sets granted by this action from the given actor. */
	void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);



	// Validation.

public:

#if WITH_EDITOR
	/** Validates the abilities and attribute sets referenced by this game feature action. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
