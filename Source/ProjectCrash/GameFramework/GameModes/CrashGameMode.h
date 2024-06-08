// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameMode.generated.h"

class UChallengerData;

/**
 * Base modular game mode for this project. This game mode uses a "game mode data" asset to define its behavior.
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

	/**
	 * Attempts to retrieve the game mode data that should be used for the current game and sends it to the game state,
	 * which takes over initialization.
	 *
	 * If no data can be found, the game cannot start, so we cancel the game and return everyone to the main menu.
	 */
	void FindGameModeData();



	// Player initialization.

public:

	/** Retrieves the given controller's Challenger from their player state, if it has been set. */
	const UChallengerData* GetChallengerDataForController(const AController* InController) const;

	/** Spawns the given controller's selected Challenger. If the game is still waiting for data for initialization,
	 * does not spawn a pawn for any controller. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;



	// Initialization states.

public:

	/** Starts listening for changes to the game state's initialization state. */
	virtual void PreInitializeComponents() override;

	/** Stops listening for changes to the game state's initialization state. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** When the game state transitions to Initializing, the game mode restarts all players. */
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;

	/** Delegate bound to when the game state's initialization state changes. The game mode's listens for changes to
	 * the game state's initialization to determine its own initialization state. */
	FDelegateHandle GameStateInitStateChangedHandle;
};
