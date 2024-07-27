// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/AsyncAction_ObserveTeamColors.h"

#include "CrashTeamAgentInterface.h"
#include "TeamStatics.h"
#include "TeamSubsystem.h"

UAsyncAction_ObserveTeamColors* UAsyncAction_ObserveTeamColors::ObserveTeamColors(UObject* InTeamAgent, bool bInLocalViewer)
{
	UAsyncAction_ObserveTeamColors* Action = nullptr;

	// Construct the new action.
	if (InTeamAgent != nullptr)
	{
		Action = NewObject<UAsyncAction_ObserveTeamColors>();
		Action->bLocalViewer = bInLocalViewer;
		Action->TeamAgentPtr = InTeamAgent;
		Action->TeamAgentInterfacePtr = TWeakInterfacePtr<ICrashTeamAgentInterface>(InTeamAgent);
		Action->RegisterWithGameInstance(InTeamAgent);
	}

	return Action;
}

void UAsyncAction_ObserveTeamColors::Activate()
{
	bool bCouldSucceed = false;
	int32 CurrentTeamIndex = INDEX_NONE;
	UTeamDisplayAsset* CurrentDisplayAsset = nullptr;

	if (ICrashTeamAgentInterface* TeamAgentInterface = TeamAgentInterfacePtr.Get())
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(TeamAgentPtr.Get(), EGetWorldErrorMode::LogAndReturnNull))
		{
			// Get current team info.
			CurrentTeamIndex = GenericTeamIdToInteger(TeamAgentInterface->GetGenericTeamId());
			CurrentDisplayAsset = UTeamStatics::GetTeamDisplayAsset(World, CurrentTeamIndex, bLocalViewer ? World->GetFirstLocalPlayerFromController()->PlayerController : nullptr);

			// Start listening for team changes.
			TeamAgentInterface->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnAgentChangedTeam);

			bCouldSucceed = true;
		}
	}

	// Broadcast initial team.
	BroadcastChange(CurrentTeamIndex, CurrentDisplayAsset);

	// If we couldn't bind to the team changes delegate, we won't be able to receive any updates.
	if (!bCouldSucceed)
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_ObserveTeamColors::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	// Stop listening for any team changes.
	if (ICrashTeamAgentInterface* TeamAgentInterface = TeamAgentInterfacePtr.Get())
	{
		TeamAgentInterface->GetTeamIdChangedDelegateChecked().RemoveAll(this);
	}
}

void UAsyncAction_ObserveTeamColors::BroadcastChange(int32 NewTeam, const UTeamDisplayAsset* DisplayAsset)
{
	UWorld* World = GEngine->GetWorldFromContextObject(TeamAgentPtr.Get(), EGetWorldErrorMode::LogAndReturnNull);
	UTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTeamSubsystem>(World);

	// Check if the team changed.
	const bool bTeamChanged = (LastBroadcastTeamId != NewTeam);

	// Stop listening for changes to the old team's display asset.
	if ((TeamSubsystem != nullptr) && bTeamChanged && (LastBroadcastTeamId != INDEX_NONE) && TeamSubsystem->DoesTeamExist(LastBroadcastTeamId))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(LastBroadcastTeamId)->RemoveAll(this);
	}

	// Broadcast the team and/or display asset change.
	const bool bTeamSet = (bTeamChanged && (NewTeam != INDEX_NONE));
	LastBroadcastTeamId = NewTeam;
	TeamColorChangedDelegate.Broadcast(bTeamSet, NewTeam, DisplayAsset);

	// Start listening for changes to the new team's display asset.
	if ((TeamSubsystem != nullptr) && bTeamChanged && (NewTeam != INDEX_NONE) && TeamSubsystem->DoesTeamExist(NewTeam))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(NewTeam)->AddDynamic(this, &ThisClass::OnTeamDisplayAssetChanged);
	}
}

void UAsyncAction_ObserveTeamColors::OnAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	// Determine the viewer for the new display asset.
	UObject* Viewer = nullptr;
	if (bLocalViewer)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(TeamAgent, EGetWorldErrorMode::LogAndReturnNull))
		{
			Viewer = World->GetFirstLocalPlayerFromController()->PlayerController;
		}
	}

	// Retrieve the new team's display asset.
	UTeamDisplayAsset* DisplayAsset = UTeamStatics::GetTeamDisplayAsset(TeamAgent, NewTeam, Viewer);

	// Broadcast the team change and the new team's display asset.
	BroadcastChange(NewTeam, DisplayAsset);
}

void UAsyncAction_ObserveTeamColors::OnTeamDisplayAssetChanged(const UTeamDisplayAsset* NewDisplayAsset)
{
	// When the team's display asset changes, broadcast the change with an unchanged team ID.
	BroadcastChange(LastBroadcastTeamId, NewDisplayAsset);
}
