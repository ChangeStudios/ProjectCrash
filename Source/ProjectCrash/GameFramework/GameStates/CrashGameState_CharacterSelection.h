// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CrashGameState_CharacterSelection.generated.h"

class UChallengerData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterSelectionCompleteSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSelectionTimeChangedSignature, int32, NewTime);

/**
 * The game state used in the "character selection" screen.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState_CharacterSelection : public AGameStateBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameState_CharacterSelection(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Starts the selection timer. */
	virtual void BeginPlay() override;



	// Character selection.

public:

	/** Ends the character selection phase and transitions all players to the match. */
	void EnterMatch();

	/** Broadcast when the character selection phase ends, either because every player locked in their Challenger or
	 * because the timer ended. */
	UPROPERTY(BlueprintAssignable, Category = "Game|Character Selection")
	FCharacterSelectionCompleteSignature CharacterSelectionCompleteDelegate;

private:

	/** Transitions all players to the match when receiving the CharacterSelectionCompleteDelegate delegate fired by
	 * EnterMatch. This allows other delegate-dependent logic to fully complete before changing levels. */
	UFUNCTION()
	void EnterMatch_Internal();



	// Timer.

public:

	/** Timer handle for managing selection time. */
	FTimerHandle SelectionTimerHandle;

	/** Broadcasts changes to the remaining selection time. */
	UPROPERTY(BlueprintAssignable, Category = "Utilities|Time")
	FSelectionTimeChangedSignature SelectionTimeChangedDelegate;

protected:

	/** The maximum total duration of the "character selection" phase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection")
	int32 TotalSelectionTime;

	/** The time remaining in the character selection phase. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_SelectionTimeRemaining)
	int32 SelectionTimeRemaining;

	/** Broadcasts the new time when the remaining selection time changes. */
	UFUNCTION()
	void OnRep_SelectionTimeRemaining();
};
