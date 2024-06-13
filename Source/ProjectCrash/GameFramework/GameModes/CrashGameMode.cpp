// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/CrashGameMode.h"

#include "CrashGameplayTags.h"
#include "Characters/ChallengerBase.h"
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

	// Send the game mode data to the game state to continue initialization.
	ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
	check(CrashGS);
	CrashGS->SetGameModeData(GameModeDataId);
}

void ACrashGameMode::OnFindGameModeDataFailed()
{
#if WITH_EDITOR
	UE_LOG(LogCrashGameMode, Error, TEXT("Unable to find game mode data. Game cannot continue."));
#endif // WITH_EDITOR

	// TODO: Cancel game and return to front-end.
}

UClass* ACrashGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// TODO: Try to retrieve pawn data from the given controller's player state.

	// TODO: Try to get the default pawn from the game mode data, if it's set.

	return nullptr;
}

void ACrashGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	/* Declare a delegate that wraps this actor's InitStateChanged function, so that it can be triggered by other
	 * features' initialization state changes. */
	FActorInitStateChangedDelegate ActorInitStateChangedDelegate = FActorInitStateChangedDelegate::CreateWeakLambda(this,
	[this](const FActorInitStateChangedParams& Params)
	{
		this->OnActorInitStateChanged(Params);
	});

	// Start listening for changes to the game state's initialization state.
	ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
	check(CrashGS);
	GameStateInitStateChangedHandle = GetComponentManager()->RegisterAndCallForActorInitState(CrashGS, CrashGS->GetFeatureName(), FGameplayTag(), ActorInitStateChangedDelegate, false);
}

void ACrashGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop listening for changes to the game state's initialization state.
	ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
	check(CrashGS);
	GetComponentManager()->UnregisterActorInitStateDelegate(CrashGS, GameStateInitStateChangedHandle);

	Super::EndPlay(EndPlayReason);
}

void ACrashGameMode::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/* When transitioning to Initializing, restart all players. At this point, the game mode data and pawn data
	 * (including Challenger data and skin data, if needed) are loaded, so players can be properly initialized when
	 * restarted. */
	if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
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
}