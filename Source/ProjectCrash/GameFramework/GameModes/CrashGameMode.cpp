// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameMode.h"

#include "Development/CrashDeveloperSettings.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/Data/CrashGameModeData.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "Player/PlayerStates/CrashPlayerState.h"


ACrashGameMode::ACrashGameMode()
{
	GameStateClass = ACrashGameState::StaticClass();
	PlayerStateClass = ACrashPlayerState::StaticClass();
	PlayerControllerClass = ACrashPlayerController::StaticClass();
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

	// TODO: Search world settings for a set game mode data asset.

	// TODO: Try dedicated server login, which will have determined its own game mode data in the command line.

	/* If the game mode data was found, use it to begin initialization. The game state takes over initialization from
	 * here. */
	if (GameModeDataId.IsValid())
	{
		UE_LOG(LogCrash, Log, TEXT("Successfully found game mode data [%s] (Source: %s)."), *GameModeDataId.ToString(), *GameModeDataSource);

		ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
		check(CrashGS);
		CrashGS->SetGameModeData(GameModeDataId);
	}
	// If the game mode data cannot be found, something has gone wrong. Return to the main menu with an error message.
	else
	{
#if WITH_EDITOR
		UE_LOG(LogCrash, Error, TEXT("Unable to find game mode data. Game cannot continue."));
#endif // WITH_EDITOR

		// TODO: Cancel game and return to front-end instead of crashing.
		UE_LOG(LogCrash, Fatal, TEXT("Unable to find game mode data."));
	}

}
