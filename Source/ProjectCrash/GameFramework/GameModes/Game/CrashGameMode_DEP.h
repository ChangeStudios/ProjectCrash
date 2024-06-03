// Copyright Samuel Reitich 2024.

#pragma once

#include "GameFramework/Data/CrashGameModeData_DEP.h"

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/Teams/CrashTeams.h"
#include "CrashGameMode_DEP.generated.h"

class ACrashPlayerState;
class APlayerStart;
class APriorityPlayerStart;
class UCrashAbilitySystemComponent;
class UGA_Death;

/** Possible state of the current match, where a match is all the gameplay that happens on a single map */
namespace CrashMatchState
{
	extern const FName InProgress_OT;	// Normal gameplay is occurring in overtime. Note that not all game-modes have overtime.
}

/**
 * The game mode used during gameplay (as opposed to menus, for example). Handles game setup, player death, and
 * victory conditions.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode_DEP : public AGameMode
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode_DEP();



	// Initialization.

public:

	virtual void PreInitializeComponents() override;

	/** Loads this game mode's data when the game starts. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Adds the Death gameplay ability to the global ability system, which will grant it to each ASC as they are
	 * created. */
	virtual void BeginPlay() override;

	/** Unloads this game mode's data when the game ends. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Player initialization.

public:

	/** Ensures that the match has not already ended before players can connect. */
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	/** Handles the new player if they join a match that is already in progress, making them a spectator if possible.
	 * If the new player had disconnected from the match, reconnect them instead. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Returns which team the given player should be assigned to, according to the game mode rules and current
	 * state. */
	virtual FCrashTeamID ChooseTeam(ACrashPlayerState* CrashPS);

	/** Spawns and initializes new players (enables input, draws the game HUD, etc.). */
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Selects the optimal player start for the given player. */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override final;

protected:

	/** Whether the given player is allowed to spawn at the given player start. By default ensures the player spawns
	 * with their team. */
	virtual bool IsPlayerStartAllowed(APlayerStart* PlayerStart, AController* Player);

	/** Returns the optimal player start for the given player within the given list of starts. By default, compares the
	 * priority of each given player start, if they are of the APrioritizedPlayerStart class. */
	virtual APlayerStart* GetPreferredStart(TArray<APlayerStart*> PlayerStarts, AController* Player);

	/** Disables players' internally cached player start, ensuring ChoosePlayerStart is always used instead. */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;



	// Match state.

// Match progress.
protected:

	/** Re-initializes the current match time and refreshes the match timer. */
	virtual void OnMatchStateSet() override;

	/** Adds "InProgress_OT" as an in-progress match state. */
	virtual bool IsMatchInProgress() const override;

// Match start.
protected:

	virtual void StartMatch() override;

	virtual void HandleMatchHasStarted() override;

// Victory condition.
public:

	/** Ends the game if IsVictoryConditionMet returns true. Call this in child game modes whenever a victory condition
	 * may have been met (e.g. when handling deaths in death matches). */
	virtual void CheckVictoryCondition();

protected:

	virtual bool IsVictoryConditionMet();

	virtual FCrashTeamID DetermineMatchWinner();

// Match end.
public:

	virtual void EndMatch() override;

	virtual void HandleLeavingMap() override;



	// Match time.

protected:

	// Handles the match time.
	FTimerHandle TimerHandle_DefaultTimer;

	// Updates the current match time, handling match state logic when the timer ends.
	void UpdateMatchTime();



	// Teams.

protected:

	/** How many teams are currently registered in the game. */
	uint8 NumTeams;



	// Game data.

public:

	/** Getter for GameModeData. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Data", Meta = (ToolTip = "Data defining various properties of this game mode."))
	UCrashGameModeData_DEP* GetGameModeData() const { return GameModeData.Get(); }

protected:

	/** Data defining various properties of this game mode, such as players' default starting lives. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Game Mode Data", Meta = (ToolTip = "Data defining various properties of this game mode, such as players' default starting lives."))
	TObjectPtr<UCrashGameModeData_DEP> GameModeData;



	// Death.

public:

	/** Handles the death (i.e. running out of health) of an actor depending on the game mode. Activates the Death
	 * gameplay ability to handle client-side death logic. */
	virtual void StartDeath(const FDeathData& DeathData);

protected:

	/** Ends the DeathAbility if one was given. */
	virtual void FinishDeath(FTimerHandle& DeathTimer, const FDeathData& DeathData);

	/** Timers used to finish an actor death a certain amount of time after it was started. We need multiple timers to
	 * handle deaths that occur simultaneously. */
	TArray<FTimerHandle> DeathTimerHandles;
};
