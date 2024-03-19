// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrashGameMode_MainMenu.generated.h"

class UUserInterfaceData;

/**
 * The primary main menu game mode. Automatically possesses any pawns active in the level.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode_MainMenu : public AGameModeBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode_MainMenu();



	// Initialization.

public:

	/** When a player enters the level, possess any pawn that can be found. */
	virtual void OnPostLogin(AController* NewPlayer) override;
};
