// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameMode.h"

#include "AbilitySystemLog.h"
#include "GameFramework/GameModes/Data/CrashGameModeData.h"
#include "EngineUtils.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Player/PriorityPlayerStart.h"
#include "Player/PlayerStates/CrashPlayerState.h"

ACrashGameMode::ACrashGameMode()
{
	NumTeams = 0;
	bDelayedStart = true;
	GameModeData = nullptr;
}

void ACrashGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Add the Death ability to the global ability system, which will grant it to each ASC as they are created.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		if (GameModeData && GameModeData->DefaultDeathAbility)
		{
			GlobalAbilitySystem->GrantGlobalAbility(GameModeData->DefaultDeathAbility);
		}
		else
		{
			ABILITY_LOG(Fatal, TEXT("ACrashGameModeBase: Game Mode [%s] does not have a default Death ability. Death logic will not function properly."), *GetName());
		}
	}
}

void ACrashGameMode::BeginPlay()
{
	UCrashAssetManager& CrashManager = UCrashAssetManager::Get();

	// Unload the main menu data when we aren't in the main menu.
	CrashManager.UnloadGameData(EGlobalGameDataType::MainMenuUIData);

	// Load this game mode's data.
	CrashManager.SyncLoadGameDataOfClass(GameModeData->GetClass(), EGlobalGameDataType::GameModeData, GameModeData, GameModeData->GetFName());
	CrashManager.SyncLoadGameDataOfClass(GameModeData->UIData->GetClass(), EGlobalGameDataType::UserInterfaceData, GameModeData->UIData, GameModeData->UIData->GetFName());

	Super::BeginPlay();
}

void ACrashGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UCrashAssetManager& CrashManager = UCrashAssetManager::Get();
	CrashManager.UnloadGameData(EGlobalGameDataType::GameModeData);
	CrashManager.UnloadGameData(EGlobalGameDataType::UserInterfaceData);
}

void ACrashGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// Ensure the game has not already ended.
	if (GameState && GameState->HasMatchEnded())
	{
		ErrorMessage = "Match has already ended.";
	}
	else
	{
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}

void ACrashGameMode::PostLogin(APlayerController* NewPlayer)
{
	/* Handle players joining matches that are already in-progress. This will usually result in the new player becoming
	 * a spectator. */
	if (NewPlayer && IsMatchInProgress())
	{
		/*
		 * TODO: Notify the player that the match is already in progress:
		 * - If the player was in the game before and is reconnecting, reconnect them.
		 * - If there is room for spectators, make the new player a spectator.
		 * - If there is no room for spectators, kick the new player out.
		 */
	}

	if (ACrashPlayerState* CrashPS = NewPlayer ? NewPlayer->GetPlayerState<ACrashPlayerState>() : nullptr)
	{
		// TODO: Override this for custom games, where teams are decided before the game begins.
		const FCrashTeamID NewTeam = ChooseTeam(CrashPS);
		UE_LOG(LogTemp, Error, TEXT("Assigned player to [%i]"), NewTeam);
		CrashPS->SetTeamID(NewTeam);
	}

	// TODO: Make sure that the player start is assigned after teams are assigned.
	Super::PostLogin(NewPlayer);

	if (NumPlayers == GameModeData->NumTeams * GameModeData->TeamSize)
	{
		StartMatch();
	}
}

FCrashTeamID ACrashGameMode::ChooseTeam(ACrashPlayerState* CrashPS)
{
	check(GameModeData);

	FCrashTeamID ReturnTeamID = FCrashTeamID::NO_TEAM;

	// TODO: Implement team tracking in game state. For now, we just increment each player's team ID to the next team.

	ReturnTeamID = NumTeams;
	NumTeams++; /* This isn't where we'll eventually update the number of teams; this function should either be const
	or be changed to something like AssignPlayerToTeam. */

	return ReturnTeamID;
}

void ACrashGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	/* TODO: Call a Client_GameStarted RPC on the connecting player.
	 *
	 * Client_GameStarted should:
	 * - Enable input
	 * - Create the HUD
	 */
}

AActor* ACrashGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	APlayerStart* DesiredStart = nullptr;

	TArray<APlayerStart*> AllowedStarts;

	// When playing from the editor, always try to use the editor's spawn-point.
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

#if WITH_EDITOR
		if (PlayerStart && PlayerStart->IsA<APlayerStartPIE>())
		{
			DesiredStart = PlayerStart;
			break;
		}
#endif // WITH_EDITOR

		// Get every allowed player start that's allowed for this player.
		if (DesiredStart == nullptr)
		{
			if (IsPlayerStartAllowed(PlayerStart, Player))
			{
				AllowedStarts.Add(PlayerStart);
			}
		}

		// Get the most preferable player start that's allowed for this player.
		if (AllowedStarts.Num() > 0)
		{
			DesiredStart = GetPreferredStart(AllowedStarts, Player);
		}
	}

	return DesiredStart ? DesiredStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool ACrashGameMode::IsPlayerStartAllowed(APlayerStart* PlayerStart, AController* Player)
{
	const ACrashPlayerState* CrashPS = Player->GetPlayerState<ACrashPlayerState>();

	// By default, players are only allowed to spawn at player starts intended for their team.
	if (const APriorityPlayerStart* PriorityPlayerStart = Cast<APriorityPlayerStart>(PlayerStart))
	{
		if (PriorityPlayerStart->GetTargetTeamID() == CrashPS->GetTeamID())
		{
			return true;
		}
	}

	return false;
}

APlayerStart* ACrashGameMode::GetPreferredStart(TArray<APlayerStart*> PlayerStarts, AController* Player)
{
	// PlayerStarts cannot be empty.
	check(PlayerStarts.Num() > 0);

	TArray<APlayerStart*> SafePlayerStarts = PlayerStarts;

	for (APlayerStart* PlayerStart : PlayerStarts)
	{
		// Remove any player starts that overlap existing pawns from SafePlayerStarts.
		TArray<AActor*> OverlappingActors;
		PlayerStart->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

		if (OverlappingActors.Num() > 0)
		{
			SafePlayerStarts.Remove(PlayerStart);
		}
	}

	APriorityPlayerStart* PreferredStart = nullptr;

	// Iterate over each safe player start. If there are no safe player starts, iterate over the player starts instead.
	for (APlayerStart* PlayerStart : (SafePlayerStarts.Num() > 0 ? SafePlayerStarts : PlayerStarts) )
	{
		if (APriorityPlayerStart* PriorityPlayerStart = Cast<APriorityPlayerStart>(PlayerStart))
		{
			// If there is no preferred start yet, initialize it to the first priority player start.
			if (PreferredStart == nullptr)
			{
				PreferredStart = PriorityPlayerStart;
				continue;
			}

			/* If the iterated player start is better than the current preferred start, update the preferred start to
			 * the better one. */
			if (PreferredStart && (PriorityPlayerStart->GetPlayerSpawnPriority() < PreferredStart->GetPlayerSpawnPriority()) )
			{
				PreferredStart = PriorityPlayerStart;
			}
		}
	}

	/* If no preferred start can be found (i.e. there are no priority starts in the given PlayerStarts array), return
	 * the first safe player start found, if valid, or the first player start found. */
	if (PreferredStart == nullptr)
	{
		if (SafePlayerStarts.Num() > 0)
		{
			return SafePlayerStarts[0];
		}
		else
		{
			return PlayerStarts[0];
		}
	}

	return PreferredStart;
}

bool ACrashGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ACrashGameMode::StartMatch()
{
	Super::StartMatch();
}

void ACrashGameMode::EndMatch()
{
	Super::EndMatch();
}

void ACrashGameMode::StartDeath(const FDeathData& DeathData)
{
	/* Send a gameplay event to the ASC of the dying actor to trigger the Death gameplay ability, which handles
	 * client-side death logic. */
	if (DeathData.DyingActorASC)
	{
		{
			FGameplayEventData Payload;
			Payload.EventTag = CrashGameplayTags::TAG_Event_Death;
			Payload.Instigator = DeathData.DamageInstigator;
			Payload.Target = DeathData.DyingActorASC->GetAvatarActor();
			Payload.OptionalObject = DeathData.DamageEffectSpec.Def;
			Payload.OptionalObject2 = DeathData.KillingDamageCauser;
			Payload.ContextHandle = DeathData.DamageEffectSpec.GetEffectContext();
			Payload.InstigatorTags = *DeathData.DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DeathData.DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DeathData.DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(DeathData.DyingActorASC, true);
			DeathData.DyingActorASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}

	// Start a timer to finish the Death after DeathDuration.
	{
		GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, FTimerDelegate::CreateLambda([this, DeathData]
		{
			FinishDeath(DeathData);
		}), GameModeData->DeathDuration, false);
	}

	// Cache the player controlling the dying actor, if it's a player-controlled pawn.
	APawn* Pawn = Cast<APawn>(DeathData.DyingActor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	UPlayer* Player = PC ? PC->Player : nullptr;
	const bool bPlayerDeath = IsValid(Player);

	UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] died. Executing [%s] death."), *DeathData.DyingActor->GetName(), *FString(bPlayerDeath ? "PLAYER PAWN" : "NON-PLAYER ACTOR"));

	/* If a player died, decrement their lives. The player state will handle the rest, and notify us if the player is
	 * now out of lives. */
	if (bPlayerDeath)
	{
		if (ACrashPlayerState* CrashPS = PC->GetPlayerState<ACrashPlayerState>())
		{
			CrashPS->DecrementLives();
		}
	}
}

void ACrashGameMode::FinishDeath(const FDeathData& DeathData)
{
	// End the Death ability when the death finishes.
	if (DeathData.DyingActorASC)
	{
		const FGameplayTagContainer DeathTags = FGameplayTagContainer(CrashGameplayTags::TAG_Event_Death);
		DeathData.DyingActorASC->CancelAbilities(&DeathTags);
	}

	// Handle respawn if the game is not over.

    UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] successfully died. Finishing death..."), *GetNameSafe(DeathData.DyingActor));
}
