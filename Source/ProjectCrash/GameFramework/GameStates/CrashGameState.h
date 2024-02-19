// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CrashGameState.generated.h"

/**
 * The game state used during gameplay (as opposed to menus, lobbies, etc.). Handles team management and game-wide
 * statistics.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AGameState
{
	GENERATED_BODY()
};
