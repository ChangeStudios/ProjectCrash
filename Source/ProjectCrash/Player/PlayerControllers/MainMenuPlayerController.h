// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CrashPlayerControllerBase.h"
#include "MainMenuPlayerController.generated.h"

/**
 * Player controller used in the main menu. Minimal class that creates the user interface and restricts gameplay to the
 * UI.
 */
UCLASS()
class PROJECTCRASH_API AMainMenuPlayerController : public ACrashPlayerControllerBase
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Initializes this player's user interface. Since the main menu is local, we don't have to worry about waiting
	 * for any kind of UI data. */
	virtual void BeginPlay() override;
};
