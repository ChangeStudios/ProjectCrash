// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/MainMenuPlayerController.h"
#include "UI/Data/UserInterfaceData.h"
#include "GameFramework/CrashAssetManager.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the player's user interface using the global main menu UI data.
	if (const UUserInterfaceData* MainMenuUIData = &UCrashAssetManager::Get().GetMainMenuUIData())
	{
		InitializeUserInterface(MainMenuUIData);
	}

	/* Restrict input to the user interface.
	 * NOTE: We may have to change this if we want to do things like using the "escape" key to go back in the menu. */
	// SetInputMode(FInputModeUIOnly());

	// TODO: Platform-specific initialization.
	bShowMouseCursor = true;
}