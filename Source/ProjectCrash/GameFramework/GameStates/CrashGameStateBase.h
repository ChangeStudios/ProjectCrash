// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameStateBase.generated.h"

class UCrashGameModeData;

/**
 * Base game state for this project. Handles initialization and universal game state components (e.g. team creator).
 */
UCLASS()
class PROJECTCRASH_API ACrashGameStateBase : public AModularGameStateBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Initialization.

// Pre-initialization.
public:

	/** Registers this actor as a feature with the initialization state framework. */
	virtual void PreInitializeComponents() override;

	/** Initializes this actor's initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters from initialization states. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

// Initialization states.
public:

	/** The name used to identify this feature (the actor) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;



	// Game mode data.

public:

	/** Updates the current game mode data path. Only works on the server. */
	void SetGameModeDataPath(TSoftObjectPtr<const UCrashGameModeData> InGameModeDataPath);

	/** Returns loaded game mode data, if it exists. Returns nullptr if the game mode data has not yet been replicated
	 * or loaded. */
	const UCrashGameModeData* GetGameModeData() const;

protected:

	/** Replicated path to current game mode data. Retrieved from the session's game options, replicated to clients for
	 * them to load. */
	UPROPERTY(ReplicatedUsing = OnRep_GameModeDataPath)
	TSoftObjectPtr<const UCrashGameModeData> GameModeDataPath;

	/** Loaded game data. */
	UPROPERTY(Transient)
	TObjectPtr<const UCrashGameModeData> GameModeData;

	/** Called when game mode data is replicated from server to clients. Instructs clients to load the data. */
	UFUNCTION()
	void OnRep_GameModeDataPath();

	/** Callback for when the game mode data finished loading. */
	UFUNCTION()
	void OnGameModeDataLoaded();
};
