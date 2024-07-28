// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Teams/TeamStatics.h"

#include "TeamDisplayAsset.h"
#include "TeamSubsystem.h"
#include "GameFramework/CrashLogging.h"

void UTeamStatics::FindTeamFromObject(const UObject* Object, bool& bIsOnTeam, int32& TeamId)
{
	bIsOnTeam = false;
	TeamId = INDEX_NONE;

	if (UWorld* World = GEngine->GetWorldFromContextObject(Object, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTeamSubsystem>())
		{
			// Attempt to retrieve a team ID associated with the given object.
			TeamId = TeamSubsystem->FindTeamFromObject(Object);

			if (TeamId != INDEX_NONE)
			{
				bIsOnTeam = true;
			}
		}
		else
		{
			UE_LOG(LogTeams, Error, TEXT("FindTeamFromObject(%s) failed: Team subsystem does not exist yet."), *GetPathNameSafe(Object));
		}
	}
}

UTeamDisplayAsset* UTeamStatics::GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId, UObject* Viewer)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTeamSubsystem>())
		{
			int32 ViewerId = INDEX_NONE;

			// Retrieve the ID of the viewer, if one was given. Otherwise, fall back to NoTeam.
			if (Viewer != nullptr)
			{
				ViewerId = TeamSubsystem->FindTeamFromObject(Viewer);
			}

			// Retrieve the effective display asset.
			return TeamSubsystem->GetTeamDisplayAsset(TeamId, ViewerId);
		}
	}

	return nullptr;
}

FLinearColor UTeamStatics::GetTeamColor(UTeamDisplayAsset* DisplayAsset, FName ColorName, FLinearColor FallbackValue)
{
	if (DisplayAsset)
	{
		// Attempt to retrieve the given color property.
		if (FLinearColor* Color = DisplayAsset->Colors.Find(ColorName))
		{
			return *Color;
		}
	}

	// Use the given fallback value if the color property doesn't exist.
	return FallbackValue;
}
