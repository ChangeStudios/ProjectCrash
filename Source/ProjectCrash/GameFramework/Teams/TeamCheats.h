// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "TeamCheats.generated.h"

/**
 * Cheats related to teams.
 */
UCLASS()
class PROJECTCRASH_API UTeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:

	/** Moves the locally controlled player to the specified team. */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamId);

	/** Logs each team currently active in the game, and their current members. */
	UFUNCTION(Exec)
	virtual void ListTeams();
};
