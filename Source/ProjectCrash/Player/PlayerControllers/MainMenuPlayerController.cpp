// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/MainMenuPlayerController.h"
#include "UI/Data/UserInterfaceData.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/Data/UserFacingMapData.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/* Load the global map data's main menu bundle. This loads map metadata exclusively used in the main menu, such as
	 * map preview textures used by the custom game lobby. */
	// TArray<FPrimaryAssetId> PrimaryAssetIds = { UCrashAssetManager::Get().GetGlobalMapData()->GetPrimaryAssetId() };
	// UCrashAssetManager::Get().ChangeBundleStateForPrimaryAssets(PrimaryAssetIds, {"MainMenu"}, {})->WaitUntilComplete(0);

	// Initialize the player's user interface using the global main menu UI data.
	// if (const UUserInterfaceData* MainMenuUIData = UCrashAssetManager::Get().GetMainMenuUIData())
	// {
	// 	InitializeUserInterface(MainMenuUIData);
	// }

	// TODO: Platform-specific initialization, e.g. hiding the "exit game" button.
	bShowMouseCursor = true;
}
