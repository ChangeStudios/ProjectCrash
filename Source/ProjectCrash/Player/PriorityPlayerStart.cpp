// Copyright Samuel Reitich. All rights reserved.


#include "Player/PriorityPlayerStart.h"

APriorityPlayerStart::APriorityPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TargetTeamID = 0;
	PlayerSpawnPriority = 0;
}
