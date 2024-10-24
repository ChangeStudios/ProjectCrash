// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/GameStateComponent.h"
#include "PlayerSpawningManagerComponent.generated.h"

class ACrashPlayerStart;

/**
 * Controls how players are spawned and respawned. Should be subclassed to define game mode-specific spawning rules.
 *
 * Default spawning rules spawn players at random player starts.
 */
UCLASS()
class PROJECTCRASH_API UPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

	friend class ACrashGameMode;

	// Construction.

public:

	/** Default constructor. */
    UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Collects every player start in the level to use when choosing player spawns. */
	virtual void InitializeComponent() override;

	/** Collects any new player starts to use when choosing player spawns. */
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);



	// Spawning.

private:

	/** Proxied by the game mode to choose the player start with this component. */
	AActor* ChoosePlayerStart(AController* Player);

protected:

	/** Virtual function that should be overridden to select the best player start depending on the game mode. */
	virtual AActor* FindBestPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts) { return nullptr; }

private:
#if WITH_EDITOR
	/** Returns the first player start created by a PIE "Play From Here" request. Returns null if the session was not
	 * started like this. */
	APlayerStart* FindPlayFromHereStart(AController* Player);
#endif // WITH_EDITOR



	// Internals.

private:

	/** Every player start in the current level, cached for convenience when choosing player spawns. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ACrashPlayerStart>> PlayerStarts_Internal;



	// Utils.

protected:

	/** Returns the first player start in the world with the given tag. */
	APlayerStart* FindFirstStartWithTag(FGameplayTag Tag);

	/** Helper for retrieving the first available player start. Used as a fallback if no player starts can be found
	 * otherwise. */
	APlayerStart* FindFirstUnoccupiedPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts) const;

	/** Finds the furthest possible player start from enemies of the given player's team. */
	AActor* FindSafestPlayerStart(AController* Player, TArray<ACrashPlayerStart*>& PlayerStarts) const;
};
