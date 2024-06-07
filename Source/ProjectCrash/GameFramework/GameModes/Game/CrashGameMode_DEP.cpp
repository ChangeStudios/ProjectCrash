// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameMode_DEP.h"

#include "AbilitySystemLog.h"
#include "EngineUtils.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GlobalGameData.h"
#include "GameFramework/GameStates/CrashGameState_DEP.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PriorityPlayerStart.h"
#include "Player/PlayerStates/CrashPlayerState.h"

namespace CrashMatchState
{
	const FName InProgress_OT = FName(TEXT("In Progress (Overtime)"));
}

ACrashGameMode_DEP::ACrashGameMode_DEP()
{
	NumTeams = 0;
	bDelayedStart = true;
	GameModeData = nullptr;
}

void ACrashGameMode_DEP::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Start the match timer.
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ACrashGameMode_DEP::UpdateMatchTime, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ACrashGameMode_DEP::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ACrashGameMode_DEP::BeginPlay()
{
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

	Super::BeginPlay();
}

void ACrashGameMode_DEP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACrashGameMode_DEP::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
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

void ACrashGameMode_DEP::PostLogin(APlayerController* NewPlayer)
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

	if (NumPlayers == GameModeData->NumTeams * GameModeData->TeamSize && !IsMatchInProgress())
	{
		StartMatch();
	}
}

FCrashTeamID ACrashGameMode_DEP::ChooseTeam(ACrashPlayerState* CrashPS)
{
	check(GameModeData);

	FCrashTeamID ReturnTeamID = FCrashTeamID::NO_TEAM;

	// TODO: Implement team tracking in game state. For now, we just increment each player's team ID to the next team.

	ReturnTeamID = NumTeams;
	NumTeams++; /* This isn't where we'll eventually update the number of teams; this function should either be const
	or be changed to something like AssignPlayerToTeam. */

	return ReturnTeamID;
}

void ACrashGameMode_DEP::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	/* TODO: Call a Client_GameStarted RPC on the connecting player.
	 *
	 * Client_GameStarted should:
	 * - Enable input
	 * - Create the HUD
	 */
}

AActor* ACrashGameMode_DEP::ChoosePlayerStart_Implementation(AController* Player)
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

bool ACrashGameMode_DEP::IsPlayerStartAllowed(APlayerStart* PlayerStart, AController* Player)
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

APlayerStart* ACrashGameMode_DEP::GetPreferredStart(TArray<APlayerStart*> PlayerStarts, AController* Player)
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

bool ACrashGameMode_DEP::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ACrashGameMode_DEP::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	// Set the current match time.
	if (ACrashGameState_DEP* const CrashGS = Cast<ACrashGameState_DEP>(GameState))
	{
		// Standard match time.
		if (MatchState == MatchState::InProgress)
		{
			CrashGS->PhaseTimeRemaining = GameModeData->MaximumMatchTime;
		}
		// Overtime match time.
		else if (MatchState == CrashMatchState::InProgress_OT)
		{
			CrashGS->PhaseTimeRemaining = GameModeData->MaximumOvertimeTime;
		}
		// Post-match match time.
		else if (MatchState == MatchState::WaitingPostMatch)
		{
			CrashGS->PhaseTimeRemaining = GameModeData->EndMatchTime;
		}
	}

	// Refresh the timer. This is required to update the timer when the global time dilation changes.
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ACrashGameMode_DEP::UpdateMatchTime, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

bool ACrashGameMode_DEP::IsMatchInProgress() const
{
	// Add "InProgress_OT" as an in-progress match state.
	if (GetMatchState() == MatchState::InProgress || GetMatchState() == CrashMatchState::InProgress_OT)
	{
		return true;
	}

	return false;
}

void ACrashGameMode_DEP::StartMatch()
{
	Super::StartMatch();
}

void ACrashGameMode_DEP::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ACrashGameMode_DEP::CheckVictoryCondition()
{
	if (IsVictoryConditionMet())
	{
		EndMatch();
	}
}

void ACrashGameMode_DEP::EndMatch()
{
	if (!IsMatchInProgress())
	{
		return;
	}

	// Apply the slow-motion effect.
	GetWorldSettings()->SetTimeDilation(UCrashAssetManager::Get().GetGlobalGameData().EndMatchTimeDilation);

	// Transition to the post-match state.
	SetMatchState(MatchState::WaitingPostMatch);

	// Notify players that the game ended.
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (ACrashPlayerState* CrashPS = (*It)->GetPlayerState<ACrashPlayerState>())
		{
			const bool bWon = DetermineMatchWinner() == CrashPS->GetTeamID();
			CrashPS->Client_HandleMatchEnded(bWon);
		}
	}
}

void ACrashGameMode_DEP::HandleLeavingMap()
{
	// Notify players that the post-match phase has ended.
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		if (ACrashPlayerState* CrashPS = (*It)->GetPlayerState<ACrashPlayerState>())
		{
			CrashPS->Client_HandleLeavingMap();
		}
	}

	Super::HandleLeavingMap();
}

void ACrashGameMode_DEP::UpdateMatchTime()
{
	ACrashGameState_DEP* const CrashGS = Cast<ACrashGameState_DEP>(GameState);
	if (CrashGS && CrashGS->PhaseTimeRemaining > 0)
	{
		// Update the current match time.
		CrashGS->PhaseTimeRemaining--;

		// Handle match state timer finishing.
		if (CrashGS->PhaseTimeRemaining <= 0)
		{
			// Start the match when the pre-match timer ends.
			if (MatchState == MatchState::WaitingToStart)
			{
				SetMatchState(MatchState::InProgress);
			}
			else if (MatchState == MatchState::InProgress)
			{
				// When the standard match time ends, start overtime if it's enabled for this game mode.
				if (GameModeData->bEnableOvertime)
				{
					SetMatchState(CrashMatchState::InProgress_OT);
				}
				// If this game mode does not have overtime, end the match when the standard match time ends.
				else
				{
					EndMatch();
				}
			}
			// End the match unconditionally when overtime ends.
			else if (MatchState == CrashMatchState::InProgress_OT)
			{
				EndMatch();
			}
			// Leave the match when the post-match timer ends.
			else if (MatchState == MatchState::WaitingPostMatch)
			{
				StartToLeaveMap();
			}
		}
	}
}

FCrashTeamID ACrashGameMode_DEP::DetermineMatchWinner()
{
	return FCrashTeamID::NO_TEAM;
}

bool ACrashGameMode_DEP::IsVictoryConditionMet()
{
	return false;
}

void ACrashGameMode_DEP::StartDeath(const FDeathData& DeathData)
{
	/* Cache the player controlling the dying actor, if it's a player-controlled pawn. We do this first because we'll
	 * lose our reference to the dying player once they unpossess their pawn. */
	const APawn* Pawn = Cast<APawn>(DeathData.DyingActor);
	const APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	const UPlayer* Player = PC ? PC->Player : nullptr;
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

	/* Send a gameplay event to the ASC of the dying actor to trigger the Death gameplay ability, which handles
	 * client-side death logic. */
	if (DeathData.DyingActorASC)
	{
		{
			FGameplayEventData Payload;
			Payload.EventTag = CrashGameplayTags::TAG_Event_Ability_Death;
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

	// Start a timer to finish the Death after DeathDuration. Create a new distinct timer to handle this death.
	FTimerHandle NewDeathTimer = DeathTimerHandles.AddDefaulted_GetRef();
	GetWorld()->GetTimerManager().SetTimer(NewDeathTimer, FTimerDelegate::CreateWeakLambda(this, [this, &NewDeathTimer, DeathData]
	{
		FinishDeath(NewDeathTimer, DeathData);
	}), GameModeData->DeathDuration, false);
}

void ACrashGameMode_DEP::FinishDeath(FTimerHandle& DeathTimer, const FDeathData& DeathData)
{
	// Destroy the timer used for this death.
	DeathTimer.Invalidate();
	if (DeathTimerHandles.Find(DeathTimer))
	{
		DeathTimerHandles.Remove(DeathTimer);
	}

	// End the Death ability when the death finishes.
	if (DeathData.DyingActorASC)
	{
		const FGameplayTagContainer DeathTags = FGameplayTagContainer(CrashGameplayTags::TAG_Event_Ability_Death);
		DeathData.DyingActorASC->CancelAbilities(&DeathTags);
	}

	// Respawn the player if the game is not over.
	if (MatchState == MatchState::InProgress)
	{
		DeathData.DyingPlayer->UnPossess();
		RestartPlayer(DeathData.DyingPlayer);
	}

    UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] successfully died. Finishing death..."), *GetNameSafe(DeathData.DyingActor));
}
