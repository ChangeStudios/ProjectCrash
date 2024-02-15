// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrashGameModeBase.generated.h"

/**
 * The base class for game modes in this project. Handles game setup, player death, and victory conditions.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	// Player death.

public:

	/** */
	void StartDeath(UPlayer* DyingPlayer);

	/** */
	void FinishDeath(UPlayer* DyingPlayer);

	/** The amount of time to wait between StartDeath and FinishDeath. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Parameters|Death")
	float DeathDuration;
};
