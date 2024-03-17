// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Data/CrashGameModeData.h"

UCrashGameModeData::UCrashGameModeData(const FObjectInitializer& ObjectInitializer) :
	MaximumMatchTime(0.0f),
	MaximumOvertimeTime(0.0f),

	NumTeams(4),
	TeamSize(1),

	StartingLives(1),

	DefaultDeathAbility(nullptr),
	DeathDuration(5.0f)
{
}
