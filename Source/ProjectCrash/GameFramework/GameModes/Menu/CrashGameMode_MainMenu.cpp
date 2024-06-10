// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/Menu/CrashGameMode_MainMenu.h"

#include "EngineUtils.h"

ACrashGameMode_MainMenu::ACrashGameMode_MainMenu()
{
	// Don't spawn any pawns automatically. The player will possess pawns that are placed in the world.
	DefaultPawnClass = nullptr;
}

void ACrashGameMode_MainMenu::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

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
