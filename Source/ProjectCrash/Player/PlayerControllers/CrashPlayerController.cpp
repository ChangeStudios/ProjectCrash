// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "AbilitySystem/CrashGameplayTags.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "GameFramework/GameModes/Game/CrashGameMode_DeathMatch.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Data/MatchUserInterfaceData.h"

void ACrashPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Broadcast that this controller's player state changed.
	PlayerStateChangedDelegate.Broadcast();
}

void ACrashPlayerController::OnMatchStateChanged(FName NewMatchState)
{
	if (NewMatchState == MatchState::WaitingToStart)
	{
		OpenCharacterSelection();
	}
}

void ACrashPlayerController::OpenCharacterSelection()
{
	if (const ACrashGameState* CrashGS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(this)))
	{
		// Load the character selection level.
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		LatentInfo.ExecutionFunction = "OnCharacterSelectionLevelLoaded";

		UGameplayStatics::LoadStreamLevel(this, CrashGS->GetCharacterSelectionLevel(), true, false, LatentInfo);

		// Create the character selection UI.
		const UCrashGameModeData* GMData = CrashGS->GetGameModeData();
		const UUserInterfaceData* UIData = GMData ? GMData->UIData : nullptr;
		if (const UMatchUserInterfaceData* MatchUIData = UIData ? Cast<UMatchUserInterfaceData>(UIData) : nullptr)
		{
			PushWidgetToLayer(MatchUIData->CharacterSelectionWidget, CrashGameplayTags::TAG_UI_Layer_Game);
		}

		SetInputMode(FInputModeUIOnly());
		bShowMouseCursor = true;
	}
}

void OnCharacterSelectionLevelLoaded()
{
	UE_LOG(LogTemp, Error, TEXT("Callback succeeded"));
}

void ACrashPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr)
	{
		/** Bind the match state handler callback to when the game's match state changes. */
		CrashGS->MatchStateChangedDelegate.AddDynamic(this, &ACrashPlayerController::OnMatchStateChanged);

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
