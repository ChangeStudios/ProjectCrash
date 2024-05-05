// Copyright Samuel Reitich 2024.


#include "GameFramework/GameStates/CrashGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ACrashGameState::BeginPlay()
{
	Super::BeginPlay();

	// Replicate the GM data from the server to the clients.
	if (HasAuthority())
	{
		const AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
		const ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
		GameModeData = CrashGM ? CrashGM->GetGameModeData() : nullptr;
	}
}

void ACrashGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TimerHandle_DefaultTimer.Invalidate();
	PhaseTimeChangedDelegate.Clear();

	Super::EndPlay(EndPlayReason);
}

void ACrashGameState::OnRep_GameModeData()
{
	// Broadcast that the game mode data has been replicated.
	GameModeDataReplicatedDelegate.Broadcast(GameModeData.Get());
}

void ACrashGameState::MulticastReliableAbilityMessageToClients_Implementation(const FCrashAbilityMessage Message)
{
	// Broadcast the given message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.MessageType, Message);
	}
}

void ACrashGameState::MulticastReliableMessageToClients_Implementation(const FCrashVerbMessage Message)
{
	// Broadcast the given message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void ACrashGameState::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	// Broadcast the new match state.
	MatchStateChangedDelegate.Broadcast(MatchState);

	// Start the appropriate timer on the server.
	if (HasAuthority())
	{
		// Standard match time.
		if (MatchState == MatchState::InProgress)
		{
			PhaseTimeRemaining = GameModeData->MaximumMatchTime;
		}
		// Overtime match time.
		else if (MatchState == CrashMatchState::InProgress_OT)
		{
			PhaseTimeRemaining = GameModeData->MaximumOvertimeTime;
		}
		// Post-match match time.
		else if (MatchState == MatchState::WaitingPostMatch)
		{
			PhaseTimeRemaining = GameModeData->EndMatchTime;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ThisClass::UpdatePhaseTime, UGameplayStatics::GetGlobalTimeDilation(this), true);
	}
}

void ACrashGameState::UpdatePhaseTime()
{
	if (PhaseTimeRemaining > 0)
	{
		PhaseTimeRemaining--;
		OnRep_PhaseTimeRemaining();
	}
	// End of phase time logic.
	else
	{
		// Match time has just ended.
		if (MatchState == MatchState::InProgress)
		{
			// Start overtime if it's enabled in this game mode.
			if (GameModeData->bEnableOvertime)
			{
				SetMatchState(CrashMatchState::InProgress_OT);
			}
			// End the game otherwise.
			else
			{
				AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
				ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
				CrashGM->EndMatch();
			}
		}
		// Always end the game when overtime ends.
		else if (MatchState == CrashMatchState::InProgress_OT)
		{
			AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
			ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
			CrashGM->EndMatch();
		}
		// Leave the map when the post-match timer ends.
		else if (MatchState == MatchState::WaitingPostMatch)
		{
			AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
			ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
			CrashGM->StartToLeaveMap();
		}
	}
}

void ACrashGameState::OnRep_PhaseTimeRemaining()
{
	// Broadcast the new time.
	PhaseTimeChangedDelegate.Broadcast(PhaseTimeRemaining);

	// Start the match when the timer reaches 0 in the pre-match phase. OnReps will handle extra logic.
	if (MatchState == MatchState::WaitingToStart && PhaseTimeRemaining == 0)
	{
		SetMatchState(MatchState::InProgress);
	}
}

void ACrashGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Since the game mode data is static, we only need to replicate its initial value.
	DOREPLIFETIME_CONDITION_NOTIFY(ACrashGameState, GameModeData, COND_InitialOnly, REPNOTIFY_Always);

	DOREPLIFETIME(ACrashGameState, PhaseTimeRemaining);
}
