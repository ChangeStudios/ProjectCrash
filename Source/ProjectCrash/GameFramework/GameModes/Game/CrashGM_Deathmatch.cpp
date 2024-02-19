// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGM_Deathmatch.h"

bool ACrashGM_Deathmatch::CheckVictoryCondition() const
{
	return Super::CheckVictoryCondition();
}

void ACrashGM_Deathmatch::StartDeath(AActor* DyingActor)
{
	Super::StartDeath(DyingActor);

	// Check the victory condition after every death. Automatically calls EndGame.
	CheckVictoryCondition();
}
