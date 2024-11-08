// Copyright Samuel Reitich. All rights reserved.

#include "GameFramework/GamePhases/GameplayDebuggerCategory_GamePhases.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GamePhaseSubsystem.h"

FGameplayDebuggerCategory_GamePhases::FGameplayDebuggerCategory_GamePhases()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_GamePhases::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(DebugActor, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGamePhaseSubsystem* GamePhaseSubsystem = World->GetSubsystem<UGamePhaseSubsystem>())
		{
			// Update the current active phases.
			for (const auto& KVP : GamePhaseSubsystem->ActivePhases)
			{
				DataPack.ActivePhases.Add(KVP.Value.PhaseTag.ToString());
			}
		}
	}
}

void FGameplayDebuggerCategory_GamePhases::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Active Game Phases:
	CanvasContext.Printf(TEXT("Active Game Phases:"));

	//		No Active Phases.
	if (DataPack.ActivePhases.IsEmpty())
	{
		CanvasContext.Printf(TEXT("\t\t{red}No active game phases."));
	}
	//		Phase
	else
	{
		for (const FString& PhaseName : DataPack.ActivePhases)
		{
			CanvasContext.Printf(TEXT("\t\t{green}%s"), *PhaseName.Replace(TEXT("GameMode.GamePhase."), TEXT("")));
		}
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_GamePhases::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_GamePhases());
}

void FGameplayDebuggerCategory_GamePhases::FRepData::Serialize(FArchive& Ar)
{
	Ar << ActivePhases;
}

#endif // WITH_GAMEPLAY_DEBUGGER