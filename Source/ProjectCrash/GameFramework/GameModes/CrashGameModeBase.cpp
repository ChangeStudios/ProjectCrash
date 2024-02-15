// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameModeBase.h"

void ACrashGameModeBase::StartDeath(UPlayer* DyingPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Death started"));
}

void ACrashGameModeBase::FinishDeath(UPlayer* DyingPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Death ended"));
}
