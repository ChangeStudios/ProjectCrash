// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameModePropertySubsystem.generated.h"

/**
 * Subsystem that manages "game mode properties."
 *
 * Game mode properties are gameplay tags mapped to a scalar value that can be queried globally. These are useful for
 * game mode-specific variables that need to be referenced by multiple systems. E.g. respawn time, game phase duration,
 * etc.
 *
 * Game mode properties can be initialized by the AddGameModeProperty game feature action. Game mode properties should
 * never change during runtime, as their changes cannot be replicated.
 *
 * Game modes only define properties relevant to them. Querying a game mode property that has not been set will always
 * yield 1.0. We use 1.0 to allow systems to use game mode properties without caring about whether they're defined. For
 * example, we can always scale knockback by the "Knockback" property, knowing it will just multiply the value by 1.0
 * if there is no registered knockback property, leaving the original knockback value unchanged.
 */
UCLASS()
class PROJECTCRASH_API UGameModePropertySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class UGameFeatureAction_AddGameModeProperties;

	// Property management.

private:

	/**
	 * Registers a game mode property with the subsystem.
	 *
	 * @return	Whether the property was successfully added. Addition will fail if the property already exists.
	 */
	bool AddGameModeProperty(FGameplayTag PropertyTag, float Value);

	/**
	 * Unregisters a game mode property from the subsystem.
	 *
	 * @return	Whether the property exists and was successfully removed.
	 */
	bool RemoveGameModeProperty(FGameplayTag Property);

public:

	/** Retrieves the given game mode property's value. Returns 1.0 if the property is not defined. */
	float GetGameModeProperty(FGameplayTag Property) const;

	/** Whether the given game mode property is registered with the game mode property subsystem. */
	bool DoesGameModePropertyExist(FGameplayTag Property) const;

	/** Retrieves all properties registered with this subsystem. Primarily used for debugging. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Game Mode Properties")
	void GetProperties(TArray<FGameplayTag>& OutProperties) { GameModeProperties.GenerateKeyArray(OutProperties); }



	// Global statics.

public:

	/** Retrieves the given game mode property's value. Returns 1.0 if the property is not defined. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Game Mode Properties", Meta = (Categories = "GameMode.Property", WorldContext = "WorldContextObject", Keywords = "gm prop find"))
	static float GetGameModeProperty(const UObject* WorldContextObject, FGameplayTag Property);

	/** Whether the given game mode property is registered with the game mode property subsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Game Mode Properties", Meta = (Categories = "GameMode.Property", WorldContext = "WorldContextObject", Keywords = "gm prop added"))
	static bool DoesGameModePropertyExist(const UObject* WorldContextObject, FGameplayTag Property);



	// Internals.

private:

	/** Internal map of game mode properties. */
	TMap<FGameplayTag, float> GameModeProperties;
};
