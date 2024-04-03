// Copyright Samuel Reitich 2024.


#include "GameFramework/GameStates/CrashGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/Game/CrashGameMode.h"
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

void ACrashGameState::OnRep_GameModeData()
{
	// Broadcast that the game mode data has been replicated.
	GameModeDataReplicatedDelegate.Broadcast(GameModeData.Get());
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
		if (MatchState == MatchState::WaitingToStart)
		{
			PhaseTimeRemaining = GameModeData->CharacterSelectionTime;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, FTimerDelegate::CreateLambda([this]
		{
			if (PhaseTimeRemaining > 0)
			{
				PhaseTimeRemaining--;
				OnRep_PhaseTimeRemaining();
			}
		}), 1.0f, true);
	}
}

void ACrashGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	UE_LOG(LogTemp, Error, TEXT("Match Started"));
}

void ACrashGameState::OnRep_PhaseTimeRemaining()
{
	// Broadcast the new time.
	PhaseTimeChangedDelegate.Broadcast(PhaseTimeRemaining);

	// Start the match when the timer reaches 0. OnReps will handle extra logic.
	if (PhaseTimeRemaining == 0)
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
