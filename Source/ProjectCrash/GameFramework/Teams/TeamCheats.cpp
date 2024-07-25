// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamCheats.h"

#include "TeamSubsystem.h"
#include "GameFramework/GameStateBase.h"
#include "Player/CrashPlayerState.h"

void UTeamCheats::SetTeam(int32 TeamId)
{
	if (UTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTeamSubsystem>(GetWorld()))
	{
		if (TeamSubsystem->DoesTeamExist(TeamId) || (IntegerToGenericTeamId(TeamId) == FGenericTeamId::NoTeam))
		{
			// Assign the local player to the given team, if it exists.
			APlayerController* PC = GetPlayerController();
			TeamSubsystem->ChangeTeamForActor(PC, TeamId);

			UE_LOG(LogConsoleResponse, Log, TEXT("------------------------------------"));
			UE_LOG(LogConsoleResponse, Log, TEXT("Assigned player [%s] to team %d."), *GetNameSafe(PC), (IntegerToGenericTeamId(TeamId) == FGenericTeamId::NoTeam) ? (int32)FGenericTeamId::NoTeam : TeamId);
			UE_LOG(LogConsoleResponse, Log, TEXT("------------------------------------"));
		}
		else
		{
			UE_LOG(LogConsoleResponse, Error, TEXT("------------------------------------"));
			UE_LOG(LogConsoleResponse, Error, TEXT("Failed to assigned player [%s] to team %d: team does not exist."), *GetNameSafe(GetPlayerController()), TeamId);
			UE_LOG(LogConsoleResponse, Error, TEXT("------------------------------------"));
		}
	}
}

void UTeamCheats::ListTeams()
{
	UE_LOG(LogConsoleResponse, Log, TEXT("------------- Current Teams: -------------"));

	if (UTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTeamSubsystem>(GetWorld()))
	{
		// Gather all players and their assigned teams, including unassigned players and no-team players.
		TMap<int32, TArray<const ACrashPlayerState*>> TeamMembers;
		TeamMembers.Reserve(TeamSubsystem->GetTeamIds().Num());

		AGameStateBase* GameState = GetWorld()->GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(PS))
			{
				// Add the player to their team's list of members.
				const int32 PlayerTeamId = CrashPS->GetTeamId();
				TeamMembers.FindOrAdd(PlayerTeamId).Add(CrashPS);
			}
		}

		// Sort by team ID, making sure to list "no team" (INDEX_NONE = -1) last.
		TeamMembers.KeySort([](const int32 A, const int32 B)
		{
			return (A < 0) ? false : (A < B);
		});

		// Log all teams and their members.
		for (const auto& KVP : TeamMembers)
		{
			// Special log for players assigned to no team.
			if ((KVP.Key == FGenericTeamId::NoTeam) || (KVP.Key == INDEX_NONE))
			{
				UE_LOG(LogConsoleResponse, Log, TEXT("No Team:"));
			}
			// Normal log for teams.
			else
			{
				UE_LOG(LogConsoleResponse, Log, TEXT("Team %d:"), KVP.Key);
			}

			// Log all members of the team or category and their current pawn.
			for (const ACrashPlayerState* CrashPS : KVP.Value)
			{
				UE_LOG(LogConsoleResponse, Log, TEXT("		%s (controlling %s)"), *GetNameSafe(CrashPS), *GetNameSafe(CrashPS->GetPawn()));
			}
		}
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("------------------------------------------"));
}
