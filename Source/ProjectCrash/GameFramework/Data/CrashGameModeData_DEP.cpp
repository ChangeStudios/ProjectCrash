// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Data/CrashGameModeData_DEP.h"

UCrashGameModeData_DEP::UCrashGameModeData_DEP(const FObjectInitializer& ObjectInitializer) :
	MaximumMatchTime(300.0f),
	bEnableOvertime(false),
	MaximumOvertimeTime(0.0f),
	EndMatchTime(5.0f),

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