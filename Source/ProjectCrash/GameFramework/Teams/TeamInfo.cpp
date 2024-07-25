// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamInfo.h"

#include "TeamSubsystem.h"
#include "Net/UnrealNetwork.h"


ATeamInfo::ATeamInfo(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	TeamId(INDEX_NONE)
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.0f;
	SetReplicatingMovement(false);
}

void ATeamInfo::BeginPlay()
{
	Super::BeginPlay();

	// Attempt to register this team with the team subsystem.
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this team from the team subsystem.
	if (TeamId != INDEX_NONE)
	{
		if (UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>())
		{
			TeamSubsystem->UnregisterTeam(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void ATeamInfo::TryRegisterWithTeamSubsystem()
{
	// Attempt to register this team with the team subsystem if it has a valid ID.
    if (TeamId != INDEX_NONE)
    {
    	UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>();
    	if (ensure(TeamSubsystem))
    	{
    		TeamSubsystem->RegisterTeam(this);
    	}
    }
}

void ATeamInfo::SetTeamId(int32 NewTeamId)
{
	check(HasAuthority());
	check(TeamId == INDEX_NONE); // Only set the team ID once.
	check(NewTeamId != INDEX_NONE);

	TeamId = NewTeamId;

	// Register this team with the team subsystem once it's assigned a valid ID.
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::OnRep_TeamId()
{
	// Register this team with the team subsystem when it's assigned a valid ID on clients.
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATeamInfo, TeamTags);
	DOREPLIFETIME_CONDITION(ATeamInfo, TeamId, COND_InitialOnly);
}