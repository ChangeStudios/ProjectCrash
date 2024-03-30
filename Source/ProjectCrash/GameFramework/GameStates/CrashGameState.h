// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "CrashGameState.generated.h"

class UCrashGameModeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameModeDataReplicatedSignature, const UCrashGameModeData*, GameModeData);

/**
 * The game state used during gameplay (as opposed to menus, lobbies, etc.). Handles team management and game-wide
 * statistics.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AGameState
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Caches the current game mode data from the server so it can be replicated to clients. */
	virtual void BeginPlay() override;



	// Game mode data.

public:

	/** Retrieves the static data for the current game mode. */
	UFUNCTION(BlueprintPure, Category = "Game Mode Data")
	const UCrashGameModeData* GetGameModeData() const { return GameModeData.Get(); }

	/** Delegate that fires when the game state receives the game mode data from the server. Allows clients to wait
	 * for the game mode data to be valid before using it. */
	UPROPERTY()
	FGameModeDataReplicatedSignature OnGameModeDataReplicated;

protected:

	/** The static data for the current game mode. Replicated to the game state from the server so it can be accessed
	 * by clients. This data is static, so it exposing it to clients doesn't pose any risk. */
	UPROPERTY(ReplicatedUsing = OnRep_GameModeData)
	TObjectPtr<UCrashGameModeData> GameModeData;

	/** Broadcasts OnGameModeDataReplicated. */
	UFUNCTION()
	void OnRep_GameModeData();



	// Messaging.

public:

	// Reliably broadcasts a verbal message to all clients. Used to replicate server-side messages.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Messaging")
	void MulticastReliableMessageToClients(const FCrashVerbMessage Message);
};
