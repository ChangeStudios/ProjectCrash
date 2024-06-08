// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameMode.h"

#include "GameFramework/GameStates/CrashGameState.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "Player/PlayerStates/CrashPlayerState.h"


ACrashGameMode::ACrashGameMode()
{
	GameStateClass = ACrashGameState::StaticClass();
	PlayerStateClass = ACrashPlayerState::StaticClass();
	PlayerControllerClass = ACrashPlayerController::StaticClass();
}
