// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AModularGameStateBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameState();



	// Initialization.

// Initializing game mode data.
public:

	/** Sets the game's current game mode data. Required to begin the game's initialization. Should not be called more
	 * than once, and should not be called after the game begins. */
	void SetGameModeData(FPrimaryAssetId GameModeDataId);



	// Initialization states.

public:

	/** The name used to identify this feature (the actor) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
};
