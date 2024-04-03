// Copyright Samuel Reitich 2024.


#include "CharacterSelectionPlayerController.h"

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
