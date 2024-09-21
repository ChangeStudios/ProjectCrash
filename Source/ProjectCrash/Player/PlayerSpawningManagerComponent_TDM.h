// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerSpawningManagerComponent.h"
#include "PlayerSpawningManagerComponent_TDM.generated.h"

/**
 * Spawns players at player starts with a matching team ID. If none can be found, spawns players as far from enemies as
 * possible.
 */
UCLASS()
class PROJECTCRASH_API UPlayerSpawningManagerComponent_TDM : public UPlayerSpawningManagerComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UPlayerSpawningManagerComponent_TDM(const FObjectInitializer& ObjectInitializer);

protected:

	/** Spawns the player at a start with their team ID. Falls back to the furthest possible start from enemies. */
	virtual AActor* FindBestPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts) override;
};
