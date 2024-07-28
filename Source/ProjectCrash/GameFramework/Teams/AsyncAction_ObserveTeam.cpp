// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/AsyncAction_ObserveTeam.h"

#include "CrashTeamAgentInterface.h"

UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::ObserveTeam(UObject* InTeamAgent)
{
	UAsyncAction_ObserveTeam* Action = nullptr;

	// Construct the new action.
	if (InTeamAgent != nullptr)
	{
		Action = NewObject<UAsyncAction_ObserveTeam>();
		Action->TeamAgentPtr = InTeamAgent;
		Action->TeamAgentInterfacePtr = TWeakInterfacePtr<ICrashTeamAgentInterface>(InTeamAgent);
		Action->RegisterWithGameInstance(InTeamAgent);
	}

	return Action;
}

void UAsyncAction_ObserveTeam::Activate()
{
	bool bCouldSucceed = false;
	int32 CurrentTeamIndex = INDEX_NONE;

	if (ICrashTeamAgentInterface* TeamAgentInterface = TeamAgentInterfacePtr.Get())
	{
		// Get the current team info.
		CurrentTeamIndex = GenericTeamIdToInteger(TeamAgentInterface->GetGenericTeamId());

		// Start listening for team changes.
		TeamAgentInterface->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnAgentChangedTeam);

		bCouldSucceed = true;
	}

	// Broadcast the initial team.
	TeamChangedDelegate.Broadcast(CurrentTeamIndex != INDEX_NONE, CurrentTeamIndex);

	// If we couldn't bind to the team changes delegate, we won't be able to receive any updates.
	if (!bCouldSucceed)
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_ObserveTeam::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	// Stop listening for team changes.
	if (ICrashTeamAgentInterface* TeamAgentInterface = TeamAgentInterfacePtr.Get())
	{
		TeamAgentInterface->GetTeamIdChangedDelegateChecked().RemoveAll(this);
	}
}

void UAsyncAction_ObserveTeam::OnAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	// Broadcast "OnTeamChanged" whenever the agent is assigned a new team.
	TeamChangedDelegate.Broadcast(NewTeam != INDEX_NONE, NewTeam);
}
