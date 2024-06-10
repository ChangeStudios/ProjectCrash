// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "CrashPlayerController.generated.h"

class ALevelSequenceActor;

/**
 * Default player controller for this project. Plays the current map's opening intro cinematic on loop, if one exists,
 * until this player is ready to be initialized.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Starts playing the current map's intro cinematic on loop until the game is ready to initialize this player. */
	virtual void BeginPlay() override;

protected:

	/** Handle to the level sequence actor currently playing the current map's intro cinematic, if it has one. */
	UPROPERTY()
	ALevelSequenceActor* IntroCinematicSequenceActor;
	
};
