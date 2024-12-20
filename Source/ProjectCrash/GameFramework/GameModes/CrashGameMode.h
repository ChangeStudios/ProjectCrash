// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "CrashGameMode.generated.h"

class UCrashGameModeData;
class UPawnData;

/** Delegate for when a player or bot joins the game and after traveling. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FGameModePlayerInitializedSignature, AGameModeBase* /* GameMode */, AController* /* NewPlayer */);

/**
 * Base modular game mode for this project. Responsible for finding the "game mode data" asset to use for the current
 * game and sending it to the game state for initialization.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



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

	/** Sends the game mode data to the game state, if it's found. The game state takes over initialization upon
	 * receiving the data. */
	void OnGameModeDataFound(const FPrimaryAssetId& GameModeDataId, const FString& GameModeDataSource);

	/** If game mode data cannot be found, the game cannot continue. Cancels the game and returns all players to the
	 * main menu. */
	void OnFindGameModeDataFailed();

	/** Called when the game state finishes fully loading the game mode (features, start-up actions, etc.). Starts any
	 * players that joined before the game mode finished loading. */
	void OnGameModeLoaded(const UCrashGameModeData* GameModeData);

	/** Returns whether the current game mode has been fully loaded. */
	bool IsGameModeLoaded() const;



	// Player spawning.

public:

	/** Uses the PlayerSpawningManagerComponent to find the best spawn for the given player. */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Disables using controllers' StartSpot to spawn them. The spawn manager should always be used instead. */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; }

	/** Don't do anything here. This is called before things like teams are set up. We wait until PostLogin to actually
	 * try to spawn the player, and we'll use the spawn manager, not StartSpot. */
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override { return true; }

	/**
	 * Tries to restart (respawn) the specified player or bot the next tick.
	 *
	 * @param Controller		The player to restart.
	 * @param bForceReset		If true, resets the controller this frame, abandoning its current pawn.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextTick(AController* Controller, bool bForceReset = false);

	/** Continuously tries to restart the player until they're spawned successfully. */
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;



	// Player initialization.

public:

	/** Prevents players from starting before the game mode finishes loading. Players prevented from starting this way
	 * will be started by OnGameModeLoaded. */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	/** Fires OnGameModePlayerInitializeDelegate after initializing a player. */
	virtual void GenericPlayerInitialization(AController* C) override;

	/** Post-login event fired when a player or bot joins the game, after they finish initialization. Also fired after
	 * traveling. */
	FGameModePlayerInitializedSignature GameModePlayerInitializedDelegate;

// Pawns.
public:

	/** Retrieves the default pawn class from the given controller's pawn data. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** Spawns a default pawn for the given player. Initializes the new pawn's pawn extension component (if it has one)
	 * with the player's pawn data. */
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

// Pawn data.
public:

	/** Attempts to retrieve the default pawn data that should be used for the given player. This is used by players to
	 * initialize their pawn data. */
	const UPawnData* FindDefaultPawnDataForPlayer(AController* Player);

	/** Retrieves the pawn data that should be used for the given controller. For players, uses their current pawn data
	 * if it's been set. Otherwise, and for non-players, uses the game mode's default pawn data. */
	const UPawnData* GetPawnDataForController(AController* InController);
};
