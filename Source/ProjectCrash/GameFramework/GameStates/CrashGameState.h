// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameModes/Game/CrashGameModeData.h"
#include "CrashGameState.generated.h"

class UCrashGameModeData;

/**
 * The game state used during gameplay (as opposed to menus, lobbies, etc.). Handles team management and game-wide
 * statistics.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AGameState
{
	GENERATED_BODY()

public:

	/** Caches the current game mode data from the server so it can be replicated to clients. */
	virtual void BeginPlay() override;

	/** Retrieves the static data for the current game mode. */
	UFUNCTION(BlueprintPure, Category = "Game Mode Data")
	const UCrashGameModeData* GetGameModeData() const { return GameModeData.Get(); }

protected:

	/** The static data for the current game mode. Replicated to the game state from the server so it can be accessed
	 * by clients. This data is static, so it exposing it to clients doesn't pose any risk. */
	UPROPERTY(Replicated)
	TSoftObjectPtr<UCrashGameModeData> GameModeData;
};
