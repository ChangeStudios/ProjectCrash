// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameFramework/GameStates/CrashGameStateBase.h"
#include "CrashGameState.generated.h"

namespace UE::GameFeatures { struct FResult; }
class ACrashPlayerState;
class UCrashGameModeData;
class UGameModeManagerComponent;

/**
 * Base modular game state for this project. This is the primary handler for game initialization, responsible for
 * loading the modular game mode data (via the UGameModeManagerComponent), and executing its startup routines
 * (executing actions, loading assets, etc.).
 *
 * @note In this framework "loading game mode data" refers to loading the game mode data asset into memory. "Loading
 * the game mode" refers to loading the data and features that comprise the current game mode (actions, game features,
 * etc.), as defined in the game mode data asset, as opposed to loading the actual AGameMode actor. In Lyra, they would
 * call the game mode the "Experience," but I want to make sure it's clear that the game mode itself is being defined
 * by the game mode data—i.e. "experiences" are just modular game modes.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AModularGameStateBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameState();



	// Initialization.

public:

	/** Registers this actor as a feature with the initialization state framework. */
	virtual void PreInitializeComponents() override;

	/** Initializes this actor's initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters this actor as an initialization state feature. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Initialization states.

public:

	/** The name used to identify this feature (the actor) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

private:

	/** Delegate used to call OnActorInitStateChanged when another actor's init state changes (e.g. a player state). */
	FActorInitStateChangedDelegate ActorInitStateChangedDelegate;

	/** Handles bound to each player state's initialization state changes. The game state's initialization state is
	 * dependent on the initialization states of each player state, so it listens for their changes. */
	TMap<ACrashPlayerState*, FDelegateHandle> PlayerStateInitStateChangedHandles;



	// Game mode.

private:

	/** Handles loading and managing the current game mode from a game mode data asset. */
	UPROPERTY()
	TObjectPtr<UGameModeManagerComponent> GameModeManagerComponent;



	// Players.

public:

	/** Starts listening for changes to the new player state's initialization state. The game state's initialization
	 * state depends on that of the player states. */
	virtual void AddPlayerState(APlayerState* PlayerState) override;



	// Utils.

private:

	/**
	 * Determines how many players should be in this game. The game will wait until this many players have logged in
	 * and been initialized before starting. Also checks for any disconnects.
	 *
	 * Uses session properties for standalone runs. Uses the editor's "Number of Players" setting during PIE.
	 */
	int32 GetNumExpectedPlayers() const;

	/**
	 * Iterates through the PlayerArray and returns the number of connected players in the given initialization
	 * state. Does not count spectators.
	 *
	 * @param bMatchStateExact		If true, will only count players in the exact given state. If false, any players in
	 *								OR PAST the given state will be counted.
	 */
	int32 GetNumPlayersInState(FGameplayTag StateToCheck, bool bMatchStateExact = false) const;
};
