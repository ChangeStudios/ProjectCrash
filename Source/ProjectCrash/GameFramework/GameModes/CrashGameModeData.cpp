// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameModeData.h"

UCrashGameModeData::UCrashGameModeData(const FObjectInitializer& ObjectInitializer)
{
	StartingLives = 1;
	DefaultDeathAbility = nullptr;
	DeathDuration = 5.0f;
}
