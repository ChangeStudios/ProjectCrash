// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "CrashGameMode_DeathMatch.generated.h"

/**
 * Game Mode for deathmatches. This game mode declares victory when every player on one team loses all of their lives.
 * This game mode works for any team size, including one player per team (i.e. free-for-all).
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode_DeathMatch : public ACrashGameMode
{
	GENERATED_BODY()

	// Death.

protected:

	/** Checks the victory condition after each death. */
	virtual void StartDeath(const FDeathData& DeathData) override;
};
