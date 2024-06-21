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
 * loading the modular game mode data and executing its startup routines (executing actions, loading assets, etc.) via
 * the GameModeManagerComponent.
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

	/** Starts listening for the game mode to be loaded via the game mode manager component. */
	virtual void PostInitializeComponents() override;

	/** Initializes this actor's initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters this actor from the initialization state framework. */
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



	// Game mode.

private:

	/** Handles loading and managing the current game mode from a game mode data asset. */
	UPROPERTY()
	TObjectPtr<UGameModeManagerComponent> GameModeManagerComponent;



	// Players.

public:

	/** Checks if all expected players have joined the game before the game can begin. */
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

	/** Gets the number of active players (all players excluding inactive players and spectators). */
	int32 GetNumActivePlayers() const;
};
