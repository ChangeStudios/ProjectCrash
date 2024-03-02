// Copyright Samuel Reitich 2024.


#include "GameFramework/GameStates/CrashGameState.h"

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
	OnGameModeDataReplicated.Broadcast(GameModeData.Get());
}

void ACrashGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Since the game mode data is static, we only need to replicate its initial value.
	DOREPLIFETIME_CONDITION_NOTIFY(ACrashGameState, GameModeData, COND_InitialOnly, REPNOTIFY_Always);
}
