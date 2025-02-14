// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GamePhases/GamePhaseCheats.h"

#include "GamePhaseAbility.h"
#include "GamePhaseSubsystem.h"
#include "Development/CrashDevelopmentStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UGamePhaseCheats::StartPhase(const FString& PhaseName)
{
#if UE_WITH_CHEAT_MANAGER
	if (GetPlayerController()->HasAuthority())
	{
		if (UGamePhaseSubsystem* GamePhaseSubsystem = UWorld::GetSubsystem<UGamePhaseSubsystem>(GetWorld()))
		{
			TSubclassOf<UGamePhaseAbility> PlayingPhase = UCrashDevelopmentStatics::FindClassByName<UGamePhaseAbility>(PhaseName);
			if (PlayingPhase != nullptr)
			{
				GamePhaseSubsystem->StartPhase(PlayingPhase);
			}
		}
	}
#endif
}
