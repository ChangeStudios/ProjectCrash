// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "CrashGM_Deathmatch.generated.h"

/**
 * Game Mode for deathmatches. This game mode declares victory when every player on one team loses all of their lives.
 * This game mode works for any team size, including one player per team (i.e. free-for-all).
 */
UCLASS()
class PROJECTCRASH_API ACrashGM_Deathmatch : public ACrashGameMode
{
	GENERATED_BODY()

	// Game state.

protected:

	/** */
	virtual bool CheckVictoryCondition() const override;



	// Death.

protected:

	/** Checks the victory condition after each death. */
	virtual void StartDeath(AActor* DyingActor) override;
};
