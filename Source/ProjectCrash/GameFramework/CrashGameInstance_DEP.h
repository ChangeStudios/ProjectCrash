// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CrashGameInstance_DEP.generated.h"

class UChallengerData;

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UCrashGameInstance_DEP : public UGameInstance
{
	GENERATED_BODY()

public:

	/** Updates the given player's selected Challenger for the match. Returns whether all players in the game state
	 * have selected Challengers. */
	bool AssignPlayerChallenger(APlayerController* Player, const UChallengerData* SelectedChallenger);

protected:

	/** Tracks which Challenger each player has selected. This data is only valid on the server. */
	UPROPERTY()
	TMap<APlayerController*, const UChallengerData*> SelectedChallengers;
};
