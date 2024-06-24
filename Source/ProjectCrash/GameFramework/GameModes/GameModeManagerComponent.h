// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "GameModeManagerComponent.generated.h"

class UCrashGameModeData;
namespace UE::GameFeatures { struct FResult; }

/** Fired when the game mode is fully loaded. */
DECLARE_MULTICAST_DELEGATE_OneParam(FCrashGameModeLoadedSignature, const UCrashGameModeData* /* GameModeData */);

/**
 * Tracks the current game mode's loading state.
 */
enum class ECrashGameModeLoadState
{
	// Game mode data has not been yet been set/received, and has not begun loading.
	Unloaded,
	// Game mode has begun loading.
	Loading,
	// Game mode is loading its dependent game features.
	LoadingGameFeatures,
	// Game mode is executing its game feature actions.
	ExecutingActions,
	// Game mode has been fully loaded.
	Loaded,
	// Game mode is currently deactivating or unloading.
	Deactivating
};



/**
 * The priority of delegates broadcast when the game mode finishes fully loading. Used to control the order in which
 * game mode loading responses execute.
 */
enum class ECrashGameModeLoadedResponsePriority
{
	// Broadcast first when the game mode finishes fully loading.
	First,
	// Broadcast after delegates with "First" priority when the game mode finishes fully loading.
	None,
	// Broadcast last, after all other delegates, when the game mode finishes loading.
	Final
};



/**
 * Game state component responsible for managing the active game mode: loading the game mode data, loading and
 * activating its plugins, and executing its actions.
 *
 * NOTE: In this framework "loading game mode data" refers to loading the game mode data asset into memory. "Loading
 * the game mode" refers to loading the data and features that comprise the current game mode (actions, game features,
 * etc.), as defined in the game mode data asset, as opposed to loading an actual AGameMode actor.
 *
 * In Lyra, they would call the game mode the "Experience," but I want to make sure it's clear that the game mode
 * itself is being defined by the game mode data—i.e. "experiences" are just modular game modes.
 */
UCLASS()
class PROJECTCRASH_API UGameModeManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()
	
	// Construction.

public:

	/** Default constructor. */
	UGameModeManagerComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Unloads the current game mode. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Game mode.

public:

	/**
	 * Sets the game's current game mode data. Required to begin the game's initialization. Should only be called on
	 * the server. Starts loading the game mode on the server; OnRep loads the game mode on clients.
	 *
	 * Should not be called more than once, and should not be called after the game begins.
	 */
	void SetCurrentGameModeData(const FPrimaryAssetId& GameModeDataId);

	/** Retrieves the current game mode data if it is fully loaded. Returns nullptr otherwise. */
	const UCrashGameModeData* GetCurrentGameModeData() const;

	/** Retrieves the current game mode data if it is fully loaded. Asserts otherwise (i.e. called too soon). */
	const UCrashGameModeData* GetCurrentGameModeDataChecked() const;

	/** Returns if the game mode data is fully loaded. */
	bool IsGameModeLoaded() const;



	// Loading.

private:

	/** Loads the appropriate bundles on the assets used by the game mode data. */
	void StartGameModeLoad();

	/** Loads all game features needed by the game mode. */
	void OnGameModeLoadComplete();

	/** Finishes loading the game mode when all game features have finished loading. */
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);

	/** Executes startup game feature actions before broadcasting that loading finished. */
	void OnGameModeFullLoadComplete();



	// Post-loading callbacks.

public:

	/** Binds a delegate with the given priority to when the game mode finishes fully loading. If the game mode is
	 * already fully loaded, immediately invokes the delegate. */
	void CallOrRegister_OnGameModeLoaded(FCrashGameModeLoadedSignature::FDelegate&& Delegate, ECrashGameModeLoadedResponsePriority Priority = ECrashGameModeLoadedResponsePriority::None);

private:

	/** Broadcast first when the current game mode has been fully loaded. */
	FCrashGameModeLoadedSignature CrashGameModeLoadedDelegate_FirstPriority;
	/** Broadcast second, after FirstPriority, when the current game mode has been fully loaded. */
	FCrashGameModeLoadedSignature CrashGameModeLoadedDelegate_NoPriority;
	/** Broadcast last when the current game mode has been fully loaded. */
	FCrashGameModeLoadedSignature CrashGameModeLoadedDelegate_FinalPriority;



	// Unloading.

private:

	/** Begins unloading the game mode by deactivating the plugins and actions loaded by the game mode. */
	void StartGameModeUnload();

	/** Attempts to invoke the final game mode unload when a pausing action finishes deactivating. */
	void OnActionDeactivationCompleted();

	/** Performs final unload of the current game mode if all pausing actions are deactivated. */
	void OnGameModeUnloadComplete();



	// Internals.

private:

	/** The game mode data being used by the current game. Should only be set once per game, when the game begins. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGameModeData)
	TObjectPtr<const UCrashGameModeData> CurrentGameModeData;

	/** Starts loading the game mode on clients when they receive the game mode data. */
	UFUNCTION()
	void OnRep_CurrentGameModeData();

	/** The current game mode's load state. */
	ECrashGameModeLoadState LoadState = ECrashGameModeLoadState::Unloaded;

	/** Tracks game features that are currently loading. All game features must be loaded before game mode load can
	 * continue. */
	int32 NumGameFeaturePluginsLoading = 0;

	/** Holds the URLs for game features that need to be loaded (or have already been loaded) for the game mode. */
	TArray<FString> GameFeaturePluginURLs;

	/** Tracks asynchronous action deactivation. */
	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;
};
