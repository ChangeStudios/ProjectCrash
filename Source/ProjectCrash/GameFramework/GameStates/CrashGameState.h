// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "CrashGameState.generated.h"

struct FCrashAbilityMessage;
class UCrashGameModeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameModeDataReplicatedSignature, const UCrashGameModeData*, GameModeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMatchStateChangedSignature, FName, NewMatchState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhaseTimeChangedSignature, uint32, NewTime);

/**
* The game state used during gameplay (as opposed to menus, lobbies, etc.). Handles the match state, team management,
* and game-wide information.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AGameState
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Caches the current game mode data from the server so it can be replicated to clients. */
	virtual void BeginPlay() override;

	/** Performs some final clean-up, like clearing the timer handle. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Game mode data.

public:

	/** Retrieves the static data for the current game mode. */
	UFUNCTION(BlueprintPure, Category = "Game Mode Data")
	const UCrashGameModeData* GetGameModeData() const { return GameModeData.Get(); }

	/** Delegate that fires when the game state receives the game mode data from the server. Allows clients to wait
	 * for the game mode data to be valid before using it. */
	UPROPERTY()
	FGameModeDataReplicatedSignature GameModeDataReplicatedDelegate;

protected:

	/** The static data for the current game mode. Replicated to the game state from the server so it can be accessed
	 * by clients. This data is static, so it exposing it to clients doesn't pose any risk. */
	UPROPERTY(ReplicatedUsing = OnRep_GameModeData)
	TObjectPtr<UCrashGameModeData> GameModeData;

	/** Broadcasts GameModeDataReplicatedDelegate. */
	UFUNCTION()
	void OnRep_GameModeData();



	// Messaging.

public:

	// Reliably broadcasts a verbal message to all clients. Used to replicate server-side messages.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Messaging")
	void MulticastReliableMessageToClients(const FCrashVerbMessage Message);

	// Reliably broadcasts an ability message to all clients. Used to replicate server-side messages.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Messaging")
	void MulticastReliableAbilityMessageToClients(const FCrashAbilityMessage Message);



	// Match state.

public:

	/** Broadcast when the game state's match state changes. */
	UPROPERTY(BlueprintAssignable)
	FMatchStateChangedSignature MatchStateChangedDelegate;

protected:

	/** Broadcasts MatchStateChangedDelegate when the match state changes. */
	virtual void OnRep_MatchState() override;



	// Timers.

public:

	/** Broadcasts changes to the current phase's remaining time. */
	UPROPERTY(BlueprintAssignable, Category = "Utilities|Time")
	FPhaseTimeChangedSignature PhaseTimeChangedDelegate;

protected:

	/** The time remaining in the current phase. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_PhaseTimeRemaining)
	int32 PhaseTimeRemaining;

	/** Updates the current phase time and triggers state-specific logic when the phase timer ends. */
	UFUNCTION()
	virtual void UpdatePhaseTime();

	/** Broadcasts the new time when the phase time changes. */
	UFUNCTION()
	void OnRep_PhaseTimeRemaining();
};
