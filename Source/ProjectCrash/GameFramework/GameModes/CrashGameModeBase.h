// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrashGameModeBase.generated.h"

class UGA_Death;

/**
 * The base class for game modes in this project. Handles game setup, player death, and victory conditions.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Adds the Death gameplay ability to the global ability system, which will grant it to each ASC as they are
	 * created. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;



	// Player death.

public:

	/** Handles the death (i.e. running out of health) of an actor by activating the Death gameplay ability. Performs
	 * client-side death logic if the actor is a pawn currently controlled by a player. */
	void StartDeath(AActor* DyingActor);

	/** */
	void FinishDeath(AActor* DyingActor);

	/** The default death ability to use in this game mode. This will be activated on ASCs when their avatars die via
	 * running out of health. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Parameters|Death")
	TSubclassOf<UGA_Death> DefaultDeathAbility;

	/** The amount of time to wait between StartDeath and FinishDeath. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Parameters|Death")
	float DeathDuration;
};
