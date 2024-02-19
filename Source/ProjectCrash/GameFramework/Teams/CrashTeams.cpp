// Copyright Samuel Reitich 2024.


#include "GameFramework/Teams/CrashTeams.h"

FCrashTeamID FCrashTeamID::GetActorTeamID(const AActor* TeamMember)
{
	const ICrashTeamMemberInterface* Team = Cast<const ICrashTeamMemberInterface>(TeamMember);

	return Team ? Team->GetTeamID() : FCrashTeamID(NO_TEAM);
}

ECrashTeamAttitude FCrashTeamID::GetAttitude(const AActor* TeamMemberA, const AActor* TeamMemberB)
{
	const ICrashTeamMemberInterface* TeamA = Cast<const ICrashTeamMemberInterface>(TeamMemberA);
	const ICrashTeamMemberInterface* TeamB = Cast<const ICrashTeamMemberInterface>(TeamMemberB);

	// If the either actor does not implement ICrashTeamMemberInterface, return Neutral.
	if (!TeamA || !TeamB)
	{
		return ECrashTeamAttitude::Neutral;
	}

	// Wrap GetAttitude using the given actor's Team IDs.
	return GetAttitude(TeamA->GetTeamID(), TeamB->GetTeamID());
}

ECrashTeamAttitude FCrashTeamID::GetAttitude(FCrashTeamID TeamA, FCrashTeamID TeamB)
{
	// Team members without a team are Neutral to all other team members.
	if (TeamA == NO_TEAM || TeamB == NO_TEAM)
	{
		return ECrashTeamAttitude::Neutral;
	}
	// Team members with the same team are Friendly.
	else if (TeamA == TeamB)
	{
		return ECrashTeamAttitude::Friendly;
	}
	// Team members with different team are Hostile.
	else
	{
		return ECrashTeamAttitude::Hostile;
	}
}



UCrashTeamMemberInterface::UCrashTeamMemberInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}