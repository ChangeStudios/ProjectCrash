// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Data/CrashGameModeData.h"

UCrashGameModeData::UCrashGameModeData(const FObjectInitializer& ObjectInitializer) :
	CharacterSelectionTime(45.0f),
	MaximumMatchTime(300.0f),
	bEnableOvertime(false),
	MaximumOvertimeTime(0.0f),

	NumTeams(4),
	TeamSize(1),

	StartingLives(1),

	DefaultDeathAbility(nullptr),
	DeathDuration(5.0f),

	UIData(nullptr),
	CustomGameActiveImage(nullptr),
	CustomGameOptionImage(nullptr)
{
}