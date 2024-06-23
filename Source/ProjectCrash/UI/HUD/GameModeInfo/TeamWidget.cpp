// Copyright Samuel Reitich. All rights reserved.


#include "TeamWidget.h"

#include "Components/Border.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/Data/GlobalGameData.h"
#include "GameFramework/Teams/CrashTeams.h"
#include "Player/CrashPlayerState.h"


void UTeamWidget::BindPlayer(APlayerState* InPlayer)
{
	check(InPlayer);
	ACrashPlayerState* BoundCrashPS = Cast<ACrashPlayerState>(InPlayer);
	check(BoundCrashPS);

	// Make sure the same player is not bound to this widget multiple times.
	if (BoundPlayers.Contains(BoundCrashPS))
	{
		return;
	}

	// Only bind players of the same team.
	if (BoundPlayers.Num() > 0)
	{
		// if (BoundCrashPS->GetTeamID() != BoundTeam)
		// {
		// 	return;
		// }
	}

	// Bind the new player.
	BoundPlayers.Add(BoundCrashPS);

	// Bind the new player's team, if they're the first player bound to this widget.
	if (BoundPlayers.Num() == 0)
	{
		// BoundTeam = BoundCrashPS->GetTeamID();
	}

	const APlayerController* LocalPC = GetOwningLocalPlayer()->PlayerController;
	const APlayerState* LocalPS = LocalPC ? LocalPC->PlayerState : nullptr;
	// const ACrashPlayerState_DEP* LocalCrashPS = LocalPS ? Cast<ACrashPlayerState_DEP>(LocalPS) : nullptr;
	// ensure(LocalCrashPS);

	const UGlobalGameData& GlobalData = UCrashAssetManager::Get().GetGlobalGameData();

	// Set the widget's border depending on the new player's team. Only do this for the first player that's bound.
	if (BoundPlayers.Num() == 1)
	{
		switch (FCrashTeamID::GetAttitude(BoundCrashPS, LocalPS))
		{
			case Friendly:
			{
				PlayerBox_Border->GetDynamicMaterial()->SetVectorParameterValue("InnerColor", GlobalData.TeamColor_Friendly.TeamUIColor);
				break;
			}
			case Hostile:
			{
				// const FCrashTeamID LocalID = LocalCrashPS->GetTeamID();
				// const FCrashTeamID CharTeamID = BoundCrashPS->GetTeamID();
					
				// PlayerBox_Border->GetDynamicMaterial()->SetVectorParameterValue("InnerColor", GlobalData.TeamColor_HostileList[(CharTeamID < LocalID) || (CharTeamID == 0) ? (int)CharTeamID : CharTeamID - 1].TeamUIColor);
				break;
			}
			default:
			{
				PlayerBox_Border->GetDynamicMaterial()->SetVectorParameterValue("InnerColor", GlobalData.TeamColor_Neutral.TeamUIColor);
			}
		}
	}
}
