// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerSpawningManagerComponent_TDM.h"

#include "CrashPlayerStart.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/Teams/TeamSubsystem.h"

UPlayerSpawningManagerComponent_TDM::UPlayerSpawningManagerComponent_TDM(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AActor* UPlayerSpawningManagerComponent_TDM::FindBestPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts)
{
	UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>();
	ACrashGameState* CrashGS = GetGameStateChecked<ACrashGameState>();

	if (!ensure(TeamSubsystem))
	{
		return nullptr;
	}

	const int32 TeamId = TeamSubsystem->FindTeamFromObject(Player);
	ACrashPlayerStart* FallbackPlayerStart = nullptr;

	// NOTE: We might want intentionally neutral players?
	if (!ensure(TeamId != INDEX_NONE))
	{
		return nullptr;
	}

	// Find a player start with a matching team ID.
	for (ACrashPlayerStart* PlayerStart : PlayerStarts)
	{
		if (PlayerStart->GetTeam() == TeamId)
		{
			// Fall back to a claimed player start if we have to.
			if (PlayerStart->IsClaimed())
			{
				FallbackPlayerStart = PlayerStart;
			}
			// Use the first usable player start with the right team.
			else if (PlayerStart->GetLocationOccupancy(Player) < EPlayerStartLocationOccupancy::Full)
			{
				return PlayerStart;
			}
		}
	}

	// The fallback start may be claimed, but at least it's the right team.
	if (FallbackPlayerStart)
	{
		return FallbackPlayerStart;
	}

	// If we can't find any player starts with a matching team, fall back to any "safe" start.
	UE_LOG(LogTeams, Error, TEXT("Tried to spawn player [%s] on team [%i], but could not find a usable player start with a matching team ID. Falling back to a random player start. Please make sure the level has enough player starts for all possible team members."), *GetNameSafe(Player), TeamId);
	return FindSafestPlayerStart(Player, PlayerStarts);
}
