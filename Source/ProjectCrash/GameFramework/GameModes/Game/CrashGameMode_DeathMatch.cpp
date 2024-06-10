// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/Game/CrashGameMode_DeathMatch.h"

#include "GameFramework/GameStateBase.h"
#include "Player/PlayerStates/CrashPlayerState_DEP.h"

bool ACrashGameMode_DeathMatch::IsVictoryConditionMet()
{
	// Build a map of each team ID, mapped to the team's total lives remaining.
	TMap<uint8, uint8> TotalTeamLives;
	for (const APlayerState* PS : GameState->PlayerArray)
	{
		if (const ACrashPlayerState_DEP* CrashPS = Cast<ACrashPlayerState_DEP>(PS))
		{
			TotalTeamLives.FindOrAdd(CrashPS->GetTeamID());
			TotalTeamLives[CrashPS->GetTeamID()] += CrashPS->GetCurrentLives();
		}
	}

	// Find how many teams are left with at least one life (i.e. how many teams are still remaining).
	uint8 TeamsWithLivesRemaining = 0;
	for (const auto TeamLives : TotalTeamLives)
	{
		if (TeamLives.Value)
		{
			TeamsWithLivesRemaining++;
		}
	}

	/* If there is only one team remaining, the game is over, with that team having won. */
	if (TeamsWithLivesRemaining == 1)
	{
		return true;
	}

	return false;
}

FCrashTeamID ACrashGameMode_DeathMatch::DetermineMatchWinner()
{
	/* Find the first player that still has lives remaining. This player will always be on the winning team in
	 * death-matches. */
	FCrashTeamID WinningTeam = FCrashTeamID::NO_TEAM;

	for (const APlayerState* PS : GameState->PlayerArray)
	{
		if (const ACrashPlayerState_DEP* CrashPS = Cast<ACrashPlayerState_DEP>(PS))
		{
			if (CrashPS->GetCurrentLives())
			{
				WinningTeam = CrashPS->GetTeamID();
				break;
			}
		}
	}

	return WinningTeam;
}

void ACrashGameMode_DeathMatch::StartDeath(const FDeathData& DeathData)
{
	Super::StartDeath(DeathData);

	// Check the victory condition after each death, since that's when victory conditions are met death-matches.
	CheckVictoryCondition();
}
