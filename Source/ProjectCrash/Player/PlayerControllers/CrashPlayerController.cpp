// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "GameFramework/Data/CrashGameModeData.h"
#include "GameFramework/GameStates/CrashGameState_DEP.h"
#include "Kismet/GameplayStatics.h"

void ACrashPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (ACrashGameState_DEP* CrashGS = GS ? Cast<ACrashGameState_DEP>(GS) : nullptr)
	{
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
			CrashGS->GameModeDataReplicatedDelegate.AddDynamic(this, &ACrashPlayerController::OnGameModeDataReplicated);
		}
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
