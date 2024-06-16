// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "CrashGameMode.generated.h"

class UCrashGameModeData;
class UPawnData;

/**
 * Base modular game mode for this project. Responsible for finding the "game mode data" asset to use for the current
 * game, and sending it to the game state for initialization.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode();



	// Initialization

public:

	/** Starts listening for the game state to load the game mode data. */
	virtual void InitGameState() override;

	/** Queues FindGameModeData for the next tick. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

// Game mode data.
public:

	/** Attempts to retrieve the game mode data that should be used for the current game. */
	void FindGameModeData();

	/**
	 * Sends the game mode data to the game state, if it's found. The game state takes over initialization upon
	 * receiving the data.
	 *
	 * Restarts all players so they can be re-initialized using the new game mode data.
	 */
	void OnGameModeDataFound(const FPrimaryAssetId& GameModeDataId, const FString& GameModeDataSource);

	/** If game mode data cannot be found, the game cannot continue. Cancels the game and returns all players to the
	 * main menu. */
	void OnFindGameModeDataFailed();

	/** Called when the game state finishes fully loading the game mode (features, start-up actions, etc.). */
	void OnGameModeLoaded(const UCrashGameModeData* GameModeData);

	/** Returns whether the current game mode has been fully loaded. */
	bool IsGameModeLoaded() const;



	// Player initialization.

public:

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/** Attempts to retrieve the pawn data that should be used for the given controller. Searches for pawn data that's
	 * been assigned to the controller (e.g. from a character selection screen). If it can't be found, uses the game
	 * mode's default pawn. */
	const UPawnData* FindPawnDataForController(AController* Controller);

	/** Spawns the given player's selected pawn, if it's been set. Otherwise, no pawn will be spawned for them. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
