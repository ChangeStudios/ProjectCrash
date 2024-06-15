// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameMode.generated.h"

class UPawnData;
class UChallengerData_DEP;

/**
 * Base modular game mode for this project. Responsible for finding the "game mode data" asset to use for the current
 * game, and sending it to the game state for initialization.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AModularGameModeBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode();



	// Initialization

// Game mode data.
public:

	/** Queues FindGameModeData for the next tick. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Attempts to retrieve the game mode data that should be used for the current game. */
	void FindGameModeData();

	/** Sends the game mode data to the game state, if it's found. The game state takes over initialization upon
	 * receiving the data. */
	void OnGameModeDataFound(const FPrimaryAssetId& GameModeDataId, const FString& GameModeDataSource);

	/** If game mode data cannot be found, the game cannot continue. Cancels the game and returns all players to the
	 * main menu. */
	void OnFindGameModeDataFailed();



	// Player initialization.

public:

	/** Attempts to retrieve the pawn data that should be used for the given controller. Searches for pawn data that's
	 * been assigned to the controller (e.g. in a character selection screen). If it can't be found, uses the game
	 * mode's default pawn. */
	const UPawnData* FindPawnDataForController(AController* Controller);

	/** Spawns the given player's selected pawn, if it's been set. Otherwise, no pawn will be spawned for them. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;



	// Initialization states.

public:

	/** Starts listening for changes to the game state's initialization state. */
	virtual void PreInitializeComponents() override;

	/** Stops listening for changes to the game state's initialization state. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** When the game state transitions to Initializing, the game mode restarts all players. */
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;

	/** Delegate bound to when the game state's initialization state changes. The game mode's listens for changes to
	 * the game state's initialization to determine its own initialization state. */
	FDelegateHandle GameStateInitStateChangedHandle;
};
