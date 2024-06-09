// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameMode.h"

#include "CrashGameplayTags.h"
#include "Characters/ChallengerBase.h"
#include "Characters/Data/ChallengerData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Development/CrashDeveloperSettings.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/CrashWorldSettings.h"
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
		UE_LOG(LogCrashGameMode, Log, TEXT("Successfully found game mode data [%s] (Source: %s)."), *GameModeDataId.ToString(), *GameModeDataSource);

		ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
		check(CrashGS);
		CrashGS->SetGameModeData(GameModeDataId);
	}
	// If the game mode data cannot be found, something has gone wrong. Return to the main menu with an error message.
	else
	{
#if WITH_EDITOR
		UE_LOG(LogCrashGameMode, Error, TEXT("Unable to find game mode data. Game cannot continue."));
#endif // WITH_EDITOR

		// TODO: Cancel game and return to front-end instead of crashing.
		UE_LOG(LogCrashGameMode, Fatal, TEXT("Unable to find game mode data."));
	}
}

const UChallengerData* ACrashGameMode::GetChallengerDataForController(const AController* InController) const
{
	// If the Challenger data has been set and loaded on the player state, use it for this controller.
	if (InController)
	{
		if (const ACrashPlayerState* CrashPS = InController->GetPlayerState<ACrashPlayerState>())
		{
			// if (const UChallengerData* ChallengerData = CrashPS->GetChallengerData())
			// {
			// 	return ChallengerData;
			// }
		}
	}

	/* If Challenger data has not been set, then the game is still waiting for data for initialization, and should not
	 * spawn a Challenger for any players. */
	return nullptr;
}

UClass* ACrashGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	/* Spawn players' selected Challengers if the game is past WaitingForData (i.e. Challenger data has been
	 * initialized). */
	ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
	check(CrashGS);
	if (GetComponentManager()->HasFeatureReachedInitState(CrashGS, CrashGS->GetFeatureName(), STATE_INITIALIZING))
	{
		if (const UChallengerData* ChallengerData = GetChallengerDataForController(InController))
		{
			if (ChallengerData->PawnClass)
			{
				return ChallengerData->PawnClass;
			}
		}
	}

	// If Challenger data has not been initialized, we don't spawn a pawn for any player.
	// NOTE: Might have to change this to Super::GetDefaultPawnClassForController_Implementation to avoid errors.
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
	ActorInitStateChangedHandle = GetComponentManager()->RegisterAndCallForActorInitState(CrashGS, CrashGS->GetFeatureName(), FGameplayTag(), ActorInitStateChangedDelegate, false);
}

void ACrashGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop listening for changes to the game state's initialization state.
	ACrashGameState* CrashGS = GetGameState<ACrashGameState>();
	check(CrashGS);
	GetComponentManager()->UnregisterActorInitStateDelegate(CrashGS, ActorInitStateChangedHandle);

	Super::EndPlay(EndPlayReason);
}

void ACrashGameMode::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	/* When transitioning to Initializing, restart all players. At this point, the game mode data, Challenger data, and
	 * skin data are all loaded, so players can be properly initialized when restarted. */
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