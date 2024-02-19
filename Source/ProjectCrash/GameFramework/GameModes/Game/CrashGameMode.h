// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "GameFramework/GameMode.h"
#include "CrashGameMode.generated.h"

class UCrashGameModeData;
class UCrashAbilitySystemComponent;
class UGA_Death;

/**
 * The game mode used during gameplay (as opposed to menus, lobbies, etc.). Handles game setup, player death, and
 * victory conditions.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AGameMode
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Adds the Death gameplay ability to the global ability system, which will grant it to each ASC as they are
	 * created. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;



	// Game data.

public:

	/** Getter for GameModeData. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game", Meta = (ToolTip = "Data defining various properties of this game mode."))
	UCrashGameModeData* GetGameModeData() const { return GameModeData; }

protected:

	/** Data defining various properties of this game mode, such as players' default starting lives. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Data")
	TObjectPtr<UCrashGameModeData> GameModeData;



	// Death.

public:

	/** Handles the death (i.e. running out of health) of an actor depending on the game mode. Activates the Death
	 * gameplay ability to handle client-side death logic. */
	virtual void StartDeath(const FDeathData& DeathData);

protected:

	/** Ends the DeathAbility if one was given. */
	virtual void FinishDeath(const FDeathData& DeathData);

	/** Timer used to finish an actor death a certain amount of time after it was started. */
	FTimerHandle DeathTimerHandle;
};
