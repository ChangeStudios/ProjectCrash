// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/Teams/TeamInfo.h"

#include "CrashGameplayTags.h"
#include "TeamSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
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

void ATeamInfo::Multicast_BroadcastTagChange_Implementation(FGameplayTag Tag, int32 Count)
{
	// Locally broadcast the received message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		// Messages don't currently support serialization, so we have to reconstruct the message we're broadcasting.
		FCrashTeamTagChangedMessage Message;
		Message.Tag = Tag;
		Message.Count = Count;
		Message.TeamId = GetTeamId();

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(CrashGameplayTags::TAG_Message_Team_TagChange, Message);
	}
}

void ATeamInfo::SetFriendlyDisplayAsset(TObjectPtr<UTeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(FriendlyDisplayAsset == nullptr);

	FriendlyDisplayAsset = NewDisplayAsset;

	/* Registering with the team subsystem refreshes our team info, which will automatically broadcast our new display
	 * asset. */
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::OnRep_FriendlyDisplayAsset(UTeamDisplayAsset* OldDisplayAsset)
{
	// Locally refresh the subsystem's team info.
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::SetTeamDisplayAsset(TObjectPtr<UTeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	/* Registering with the team subsystem refreshes our team info, which will automatically broadcast our new display
	 * asset. */
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::OnRep_TeamDisplayAsset(UTeamDisplayAsset* OldDisplayAsset)
{
	// Locally refresh the subsystem's team info.
	TryRegisterWithTeamSubsystem();
}

void ATeamInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATeamInfo, TeamTags);
	DOREPLIFETIME_CONDITION(ATeamInfo, TeamId, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ATeamInfo, FriendlyDisplayAsset, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ATeamInfo, TeamDisplayAsset, COND_InitialOnly);
}
