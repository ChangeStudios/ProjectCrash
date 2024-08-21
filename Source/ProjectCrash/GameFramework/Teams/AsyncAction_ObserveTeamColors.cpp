// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/AsyncAction_ObserveTeamColors.h"

#include "CrashTeamAgentInterface.h"
#include "TeamStatics.h"
#include "TeamSubsystem.h"
#include "GameFramework/CrashLogging.h"

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
			// Get the current team info.
			CurrentTeamIndex = GenericTeamIdToInteger(TeamAgentInterface->GetGenericTeamId());
			CurrentDisplayAsset = GetRelevantDisplayAsset(CurrentTeamIndex);

			// Start listening for team changes.
			TeamAgentInterface->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnAgentChangedTeam);

			/* If we're using local viewing, we also need to listen for when the viewer (the local player controller)
			 * initializes or changes their team, so the effective display asset remains accurate to the viewer. */
			if (bLocalViewer && TeamAgentPtr.Get())
			{
				if (ICrashTeamAgentInterface* ViewerAgentInterface = Cast<ICrashTeamAgentInterface>(GetViewer(TeamAgentPtr.Get())))
				{
					ViewerAgentInterface->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnViewerChangedTeam);
				}
			}

			bCouldSucceed = true;
		}
	}

	// Broadcast the initial team.
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

	// Stop listening for team changes.
	if (ICrashTeamAgentInterface* TeamAgentInterface = TeamAgentInterfacePtr.Get())
	{
		TeamAgentInterface->GetTeamIdChangedDelegateChecked().RemoveAll(this);
	}

	if (bLocalViewer && TeamAgentPtr.Get())
	{
		if (ICrashTeamAgentInterface* ViewerAgentInterface = Cast<ICrashTeamAgentInterface>(GetViewer(TeamAgentPtr.Get())))
		{
			ViewerAgentInterface->GetTeamIdChangedDelegateChecked().RemoveAll(this);
		}
	}
}

void UAsyncAction_ObserveTeamColors::BroadcastChange(int32 NewTeam, const UTeamDisplayAsset* DisplayAsset)
{
	UWorld* World = GEngine->GetWorldFromContextObject(TeamAgentPtr.Get(), EGetWorldErrorMode::LogAndReturnNull);
	UTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<UTeamSubsystem>(World);

	// Check if the team changed.
	const bool bTeamChanged = (LastBroadcastTeamId != NewTeam);

	// Stop listening for changes to the old team's display asset.
	if ((TeamSubsystem != nullptr) && bTeamChanged && (LastBroadcastTeamId != INDEX_NONE))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(LastBroadcastTeamId).RemoveAll(this);
	}

	// Broadcast the team and/or display asset change.
	LastBroadcastTeamId = NewTeam;
	TeamColorChangedDelegate.Broadcast((NewTeam != INDEX_NONE), NewTeam, DisplayAsset);

	// Start listening for changes to the new team's display asset.
	if ((TeamSubsystem != nullptr) && bTeamChanged && (NewTeam != INDEX_NONE))
	{
		TeamSubsystem->GetTeamDisplayAssetChangedDelegate(NewTeam).AddDynamic(this, &ThisClass::OnTeamDisplayAssetChanged);
	}
}

void UAsyncAction_ObserveTeamColors::OnAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	// Broadcast the team change and the new team's display asset.
	BroadcastChange(NewTeam, GetRelevantDisplayAsset(NewTeam));
}

void UAsyncAction_ObserveTeamColors::OnTeamDisplayAssetChanged(int32 TeamId)
{
	// When the team's display asset changes, broadcast the change with an unchanged team ID.
	BroadcastChange(LastBroadcastTeamId, GetRelevantDisplayAsset(LastBroadcastTeamId));
}

void UAsyncAction_ObserveTeamColors::OnViewerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ensure(bLocalViewer);

	// Our observed agent's team hasn't changed, but GetRelevantDisplayAsset will use our viewer's new team.
	BroadcastChange(LastBroadcastTeamId, GetRelevantDisplayAsset(LastBroadcastTeamId));
}

UTeamDisplayAsset* UAsyncAction_ObserveTeamColors::GetRelevantDisplayAsset(int32 TeamId)
{
	// Get the local player as the viewer.
	ensure(TeamAgentPtr.Get());
	UObject* Viewer = GetViewer(TeamAgentPtr.Get());

	// Use the team subsystem to determine the relevant display asset.
	return UTeamStatics::GetTeamDisplayAsset(TeamAgentPtr.Get(), TeamId, Viewer);
}

UObject* UAsyncAction_ObserveTeamColors::GetViewer(UObject* WorldContextObject)
{
	UObject* Viewer = nullptr;

	// No viewer is used if bLocalViewer is false.
	if (bLocalViewer)
	{
		// Retrieve the world to get the local player.
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			// Safely retrieve the local player.
			if (ULocalPlayer* LP = World->GetFirstLocalPlayerFromController())
			{
				// If the local player was found with a PC search, then the PC will be valid.
				Viewer = LP->PlayerController;
			}
		}
	}

	return Viewer;
}
