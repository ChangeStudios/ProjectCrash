// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/GlobalLayeredWidget.h"
#include "UI/Widgets/Utils/SlottedEntryBox.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void ACrashPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Broadcast that this controller's player state changed.
	PlayerStateChangedDelegate.Broadcast();
}

void ACrashPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;

	/* If the game mode data is already valid (i.e. this is a listen server's player), immediately initialize this
	 * player's UI. */
	if (CrashGS->GetGameModeData())
	{
		InitializeUserInterface(CrashGS->GetGameModeData()->UIData);
	}
	/* If the game mode data has not been replicated yet, assign InitializeUserInterface as a callback to when it
	 * becomes available. */
	else
	{
		CrashGS->OnGameModeDataReplicated.AddDynamic(this, &ACrashPlayerController::OnGameModeDataReplicated);
	}

	// Set the input mode.
	SetInputMode(FInputModeGameOnly());

	// TODO: Platform-specific initialization.
	bShowMouseCursor = false;
}

void ACrashPlayerController::OnGameModeDataReplicated(const UCrashGameModeData* GameModeData)
{
	if (GameModeData->UIData)
	{
		InitializeUserInterface(GameModeData->UIData);
	}
}
