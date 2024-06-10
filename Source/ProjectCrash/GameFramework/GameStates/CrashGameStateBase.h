// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameStateBase.generated.h"

class UCrashGameModeData;

/** Delegate for broadcasting when game mode data is received and loaded locally. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameModeDataLoadedSignature, const UCrashGameModeData*, GameModeData);

/**
 * Base game state for this project. Handles initialization and universal game state components (e.g. team creation).
 */
UCLASS()
class PROJECTCRASH_API ACrashGameStateBase : public AModularGameStateBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Initialization.

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

protected:

	/** Wraps this actor's OnActorInitStateChanged to call it when receiving changes to another actor's init state. */ 
	FActorInitStateChangedDelegate ActorInitStateChangedDelegate;

	/** Handles bound to other actor initialization state changes, e.g. each player state, and the corresponding
	 * actors. Used to unregister from their init state changes when the game ends. */
	TMap<TObjectPtr<AActor>, FDelegateHandle> ActorInitStateChangedHandles;



	// Players.

public:

	/** Starts listening for changes to new player states' initialization states when a player joins the game. The
	 * game state's initialization state relies on the init state of the connected player states. */
	virtual void AddPlayerState(APlayerState* PlayerState) override;



	// Game mode data.

public:

	/** Updates the current game mode data. */
	void SetGameModeData(FPrimaryAssetId GameModeDataId);

	/** Returns loaded game mode data, if it exists. Returns nullptr if the game mode data has not yet been replicated
	 * or loaded. */
	const UCrashGameModeData* GetGameModeData() const;

	/** Calls the given delegate if the game mode data is loaded. If not, binds the delegate to when the data is
	 * loaded. */
	void CallOrRegister_OnGameModeDataLoaded(FGameModeDataLoadedSignature::FDelegate&& GameModeDataLoadedDelegate);


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

	/** Fired when the game mode data finishes loading locally, after being retrieved on or replicated from the server.
	 * Used by other classes that need to wait for game mode data to be loaded. */
	FGameModeDataLoadedSignature GameModeDataLoadedDelegate;
};
