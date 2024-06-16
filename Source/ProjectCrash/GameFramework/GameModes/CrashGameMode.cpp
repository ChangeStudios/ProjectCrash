// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameMode.h"

#include "CrashGameplayTags.h"
#include "GameModeManagerComponent.h"
#include "Characters/ChallengerBase.h"
#include "Characters/Data/PawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Development/CrashDeveloperSettings.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/CrashWorldSettings.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "Player/PlayerStates/CrashPlayerState.h"

ACrashGameMode::ACrashGameMode()
{
	GameStateClass = ACrashGameState::StaticClass();
	PlayerStateClass = ACrashPlayerState::StaticClass();
	// PlayerControllerClass = ACrashPlayerController::StaticClass();
}

void ACrashGameMode::InitGameState()
{
	Super::InitGameState();

	// Start listening for the game mode to be fully loaded.
	UGameModeManagerComponent* GameModeManagerComponent = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManagerComponent);
	GameModeManagerComponent->CallOrRegister_OnGameModeLoaded(
		FCrashGameModeLoadedSignature::FDelegate::CreateUObject(this,&ThisClass::OnGameModeLoaded),
		ECrashGameModeLoadedResponsePriority::Final);
}

void ACrashGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Call FindGameModeData on the next tick to give time to initialize startup settings.
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::FindGameModeData);
}

void ACrashGameMode::FindGameModeData()
{
	/**
	 * Attempt to retrieve game mode data in the following methods. The first game mode data succcessfully retrieved
	 * will be used.
	 *
	 *		- Game options, using the "GameModeData" argument.
	 *		- Developer settings, if in PIE.
	 *		- Attempts to retrieve a default game mode from the world settings. E.g. front-end levels.
	 *		- If on a dedicated server, tries to log in and read the desired game mode data from the command line using
	 *		  the "GameModeData" argument.
	 *		- Cancels the game and returns to the main menu if no game mode data can be found.
	 */

	FPrimaryAssetId GameModeDataId;
	FString GameModeDataSource;
	UWorld* World = GetWorld();

	// Search the game options for game mode data. Used when hosting a custom game.
	if (!GameModeDataId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("GameModeData")))
	{
		const FString GameModeDataFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("GameModeData"));
		GameModeDataId = FPrimaryAssetId(FPrimaryAssetType(UCrashGameModeData::StaticClass()->GetFName()), FName(*GameModeDataFromOptions));
		GameModeDataSource = TEXT("Game Options");
	}

	// If in PIE, override the game mode data using developer settings.
	if (!GameModeDataId.IsValid() && World->IsPlayInEditor())
	{
		if (const UCrashDeveloperSettings* DeveloperSettings = GetDefault<UCrashDeveloperSettings>())
		{
			// Only override the game mode data if an overriding game mode data asset has been set.
			if (DeveloperSettings->GameModeDataOverride.IsValid())
			{
				GameModeDataId = DeveloperSettings->GameModeDataOverride;
				GameModeDataSource = TEXT("Developer Settings");
			}
		}
	}

	// Search world settings for a set game mode data asset.
	if (!GameModeDataId.IsValid())
	{
		if (ACrashWorldSettings* CrashWorldSettings = Cast<ACrashWorldSettings>(GetWorldSettings()))
		{
			GameModeDataId = CrashWorldSettings->GetDefaultGameModeData();
			GameModeDataSource = TEXT("World Settings");
		}
	}

	// TODO: Try dedicated server login, which will have determined its own game mode data in the command line.

	/* If the game mode data was found, use it to begin initialization. The game state takes over initialization from
	 * here. */
	if (GameModeDataId.IsValid())
	{
		// Send the game mode data to the game state, which will take over initialization.
		OnGameModeDataFound(GameModeDataId, GameModeDataSource);
	}
	// If the game mode data cannot be found, something has gone wrong. Return to the main menu with an error message.
	else
	{
		OnFindGameModeDataFailed();
	}
}

void ACrashGameMode::OnGameModeDataFound(const FPrimaryAssetId& GameModeDataId, const FString& GameModeDataSource)
{
	UE_LOG(LogCrashGameMode, Log, TEXT("Successfully found game mode data [%s] (Source: %s)."), *GameModeDataId.ToString(), *GameModeDataSource);

	// Send the game mode data to the game state's game mode manager component to continue initialization.
	UGameModeManagerComponent* GameModeManagerComponent = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManagerComponent);
	GameModeManagerComponent->SetCurrentGameModeData(GameModeDataId);
}

void ACrashGameMode::OnFindGameModeDataFailed()
{
#if WITH_EDITOR
	UE_LOG(LogCrashGameMode, Error, TEXT("Unable to find game mode data. Game cannot continue."));
	return;
#endif // WITH_EDITOR

	// TODO: Cancel game and return to front-end.
}

void ACrashGameMode::OnGameModeLoaded(const UCrashGameModeData* GameModeData)
{
	/* Restart all players, so they can be re-initialized using the new game mode data. This usually only affects the
	 * listen server player, since everyone else typically won't even be in the game when the game mode finishes
	 * loading on the server. */
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

bool ACrashGameMode::IsGameModeLoaded() const
{
	// Check if the game mode manager component has finished fully loading the current game mode.
	check(GameState);
	UGameModeManagerComponent* GameModeManagerComponent = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManagerComponent);

	return GameModeManagerComponent->IsGameModeLoaded();
}

void ACrashGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	/**  */
	if (IsGameModeLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

const UPawnData* ACrashGameMode::FindPawnDataForController(AController* Controller)
{
	check(GameState);
	UGameModeManagerComponent* GameModeManagerComponent = GameState->FindComponentByClass<UGameModeManagerComponent>();
	check(GameModeManagerComponent);

	// Always return null if the game mode has not been loaded yet.
	if (!GameModeManagerComponent->IsGameModeLoaded())
	{
		return nullptr;
	}

	/**
	 * If the game mode is loaded, attempt to retrieve pawn data in the following methods. The first pawn data
	 * succcessfully retrieved will be used.
	 *
	 *		- Session/game settings, if a pawn has been assigned to the specific player.
	 *		- Developer settings, if in PIE.
	 *		- Game mode's default pawn as defined in the GM data.
	 */

	const UPawnData* NewPawnData = nullptr;
	FString PawnDataSource;
	UWorld* World = GetWorld();

	// TODO: Check for pawn data assigned to the given player in the session/game's settings.

	// If in PIE, override the pawn data using developer settings.
	if (!NewPawnData && World->IsPlayInEditor())
	{
		if (const UCrashDeveloperSettings* DeveloperSettings = GetDefault<UCrashDeveloperSettings>())
		{
			// Only override the pawn data if an overriding pawn data asset has been set.
			if (DeveloperSettings->PawnDataOverride)
			{
				NewPawnData = DeveloperSettings->PawnDataOverride;
				PawnDataSource = TEXT("Developer Settings");
			}
		}
	}

	// Use the game mode's default pawn data.
	if (!NewPawnData)
	{
		const UCrashGameModeData* GameModeData = GameModeManagerComponent->GetCurrentGameModeDataChecked();

		NewPawnData = GameModeData->DefaultPawn;
		PawnDataSource = TEXT("Game Mode Default Pawn");
	}

	// Use the found pawn data.
	if (NewPawnData != nullptr)
	{
		UE_LOG(LogCrashGameMode, Log, TEXT("Successfully found pawn data [%s] for player [%s] (Source: %s)."), *GetNameSafe(NewPawnData), *GetNameSafe(Controller), *PawnDataSource);
		return NewPawnData;
	}

	// If no pawn data was found, something is wrong. This function was likely called too early.
	UE_LOG(LogCrashGameMode, Error, TEXT("Game mode unable to retrieve pawn data for player [%s]."), *GetNameSafe(Controller));
	return nullptr;
}

UClass* ACrashGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// If the player's pawn data has been set, use their pawn data.
	if (InController)
	{
		if (const ACrashPlayerState* CrashPS = InController->GetPlayerState<ACrashPlayerState>())
		{
			if (const UPawnData* PawnData = CrashPS->GetPawnData<UPawnData>())
			{
				if (PawnData->PawnClass)
				{
					return PawnData->PawnClass;
				}
			}
		}
	}

	// If the player's pawn data has not been set yet, do not spawn a pawn for them.
	return nullptr;
}