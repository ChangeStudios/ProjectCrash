// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameModeData.h"

UCrashGameModeData::UCrashGameModeData(const FObjectInitializer& ObjectInitializer)
{
	TeamSize = 1;
	StartingLives = 1;
	DefaultDeathAbility = nullptr;
	DeathDuration = 5.0f;
}
