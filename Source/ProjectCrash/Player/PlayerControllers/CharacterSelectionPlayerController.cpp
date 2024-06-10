// Copyright Samuel Reitich. All rights reserved.


#include "CharacterSelectionPlayerController.h"

#include "GameFramework/CrashGameInstance_DEP.h"
#include "GameFramework/GameStates/CrashGameState_CharacterSelection.h"
#include "Kismet/GameplayStatics.h"

void ACharacterSelectionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Create the UI.
	InitializeUserInterface(CharacterSelectionUIData);

	/* Restrict input to the user interface.
	 * NOTE: We may have to change this if we want to do things like using the "escape" key to go back in the menu. */
	SetInputMode(FInputModeUIOnly());

	// TODO: Platform-specific initialization.
	bShowMouseCursor = true;
}

void ACharacterSelectionPlayerController::Server_LockInChallenger_Implementation(const UChallengerData* SelectedChallenger)
{
	check(SelectedChallenger);

	if (UCrashGameInstance_DEP* CrashGI = Cast<UCrashGameInstance_DEP>(GetGameInstance()))
	{
		// Assign this player's Challenger, checking if they are the final player to do so.
		if (CrashGI->AssignPlayerChallenger(this, SelectedChallenger))
		{
			// If the game instance says all players are ready, tell the game state to start the game.
			AGameStateBase* GS = UGameplayStatics::GetGameState(this);
			if (ACrashGameState_CharacterSelection* CharSelectionGS = GS ? Cast<ACrashGameState_CharacterSelection>(GS) : nullptr)
			{
				CharSelectionGS->EnterMatch();
			}
		}
	}
}