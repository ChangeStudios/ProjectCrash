// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CrashPlayerControllerBase.h"
#include "CrashPlayerController.generated.h"

class UCrashGameModeData;
class APlayerState;
class USlottedEntryBox;
class UGlobalLayeredWidget;

/** Delegate for broadcasting when this controller's player state changes. */
DECLARE_MULTICAST_DELEGATE(FPlayerStateChangeSignature);

/**
 * Base player controller used during gameplay (as opposed to menus, for example). Handles initialization and
 * manipulation of the user interface.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerController : public ACrashPlayerControllerBase
{  
	GENERATED_BODY()

	// Initialization.

public:

	/** Initializes this player's user interface if the game mode data is valid. Otherwise, assigns a callback to
	 * initialize the UI when the game mode data becomes available. */
	virtual void BeginPlay() override;

	/** Callback for when the game mode data is first replicated. Wraps InitializeUserInterface. */
	UFUNCTION()
	void OnGameModeDataReplicated(const UCrashGameModeData* GameModeData);

	/** Broadcasts PlayerStateChangedDelegate when this controller's player state has been changed. */
	virtual void OnRep_PlayerState() override;

	/** Broadcast when this controller's player state is changed. */
	FPlayerStateChangeSignature PlayerStateChangedDelegate;
};