// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrashGameMode_CharacterSelection.generated.h"

/**
 * The game mode for the character selection pre-game phase.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode_CharacterSelection : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Possesses the first pawn that can be found in the world when the player logs in. */
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
