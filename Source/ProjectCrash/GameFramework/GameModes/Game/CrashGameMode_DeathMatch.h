// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModes/Game/CrashGameMode_DEP.h"
#include "CrashGameMode_DeathMatch.generated.h"

/**
 * Game Mode for deathmatches. This game mode declares victory when every player on one team loses all of their lives.
 * This game mode works for any team size, including one player per team (i.e. free-for-all).
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode_DeathMatch : public ACrashGameMode_DEP
{
	GENERATED_BODY()

	// Game mode logic.

protected:

	/** Returns true if there is only one team remaining that has any lives left. */
	virtual bool IsVictoryConditionMet() override;



	// Utils.

protected:

	/** Returns the first team with any lives remaining. */
	virtual FCrashTeamID DetermineMatchWinner() override;



	// Death.

protected:

	/** Checks the victory condition after each death. */
	virtual void StartDeath(const FDeathData& DeathData) override;
};
