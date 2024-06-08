// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AModularGameModeBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode();



	// Initialization.

// Initializing game mode data.
public:

	/** Queues FindGameModeData for the next tick. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Attempts to retrieve the game mode data that should be used for the current game. If no data can be found, the
	 * game cannot start, so we cancel the game and return everyone to the main menu. */
	void FindGameModeData();
};
