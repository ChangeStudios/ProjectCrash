// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameMode_CharacterSelection.h"

#include "EngineUtils.h"

void ACrashGameMode_CharacterSelection::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Try to possess the first pawn that can be found in the world.
	const UWorld* World = GetWorld();
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* Pawn = *It;
		if (IsValid(Pawn))
		{
			NewPlayer->Possess(Pawn);
		}
	}
}
