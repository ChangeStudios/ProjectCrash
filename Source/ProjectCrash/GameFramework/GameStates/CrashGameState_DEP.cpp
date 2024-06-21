// Copyright Samuel Reitich 2024.


#include "GameFramework/GameStates/CrashGameState_DEP.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/Game/CrashGameMode_DEP.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ACrashGameState_DEP::BeginPlay()
{
	Super::BeginPlay();

	// Replicate the GM data from the server to the clients.
	if (HasAuthority())
	{
		const AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
		const ACrashGameMode_DEP* CrashGM = GM ? Cast<ACrashGameMode_DEP>(GM) : nullptr;
		GameModeData = CrashGM ? CrashGM->GetGameModeData() : nullptr;
	}
}

void ACrashGameState_DEP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACrashGameState_DEP::OnRep_GameModeData()
{
	// Broadcast that the game mode data has been replicated.
	GameModeDataReplicatedDelegate.Broadcast(GameModeData.Get());
}

void ACrashGameState_DEP::MulticastReliableAbilityMessageToClients_Implementation(const FCrashAbilityMessage Message)
{
	// Broadcast the given message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.MessageType, Message);
	}
}

void ACrashGameState_DEP::MulticastReliableMessageToClients_Implementation(const FCrashVerbMessage Message)
{
	// Broadcast the given message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void ACrashGameState_DEP::OnRep_MatchState()
{
	Super::OnRep_MatchState();

	// Broadcast the new match state.
	MatchStateChangedDelegate.Broadcast(MatchState);
}

void ACrashGameState_DEP::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Since the game mode data is static, we only need to replicate its initial value.
	DOREPLIFETIME_CONDITION_NOTIFY(ACrashGameState_DEP, GameModeData, COND_InitialOnly, REPNOTIFY_Always);

	DOREPLIFETIME(ACrashGameState_DEP, PhaseTimeRemaining);
}
