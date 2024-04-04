// Copyright Samuel Reitich 2024.


#include "CrashGameState_CharacterSelection.h"

#include "Net/UnrealNetwork.h"

ACrashGameState_CharacterSelection::ACrashGameState_CharacterSelection(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	TotalSelectionTime(45.0f),
	SelectionTimeRemaining(0.0f)
{
}

void ACrashGameState_CharacterSelection::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		/* Bind match transition logic to when the selection phase ends. This allows the delegates other bindings to
		 * properly execute before the level transition. */
		CharacterSelectionCompleteDelegate.AddDynamic(this, &ACrashGameState_CharacterSelection::EnterMatch_Internal);
		
		// Initialize character selection phase time.
		SelectionTimeRemaining = TotalSelectionTime;

		// Start the character selection phase timer.
		GetWorldTimerManager().SetTimer(SelectionTimerHandle, FTimerDelegate::CreateLambda([this]
		{
			if (SelectionTimeRemaining > 0)
			{
				SelectionTimeRemaining--;
				OnRep_SelectionTimeRemaining();
			}
		}), 1.0f, true);
	}
}

void ACrashGameState_CharacterSelection::EnterMatch()
{
	// Disable the timer.
    GetWorldTimerManager().ClearTimer(SelectionTimerHandle);

	// Broadcast that the character selection phase has completed.
	CharacterSelectionCompleteDelegate.Broadcast();
}

void ACrashGameState_CharacterSelection::EnterMatch_Internal()
{
	// TODO: Get map from the session info.

	// Travel to the match map.
	GetWorld()->ServerTravel("Level1_Game", false);
}

void ACrashGameState_CharacterSelection::OnRep_SelectionTimeRemaining()
{
	// Broadcast the new time.
	SelectionTimeChangedDelegate.Broadcast(SelectionTimeRemaining);

	// Start the match when the timer reaches 0.
	if (SelectionTimeRemaining == 0)
	{
		EnterMatch();
	}
}

void ACrashGameState_CharacterSelection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashGameState_CharacterSelection, SelectionTimeRemaining);
}
