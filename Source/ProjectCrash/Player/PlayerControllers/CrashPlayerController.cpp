// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/CrashPlayerController.h"

#include "GameFramework/CrashWorldSettings.h"
#include "LevelSequencePlayer.h"

void ACrashPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/* Play this map's intro cinematic on loop, if it has one. This will be stopped when the game is ready to
	 * initialize this player. */
	if (ACrashWorldSettings* CrashWorldSettings = Cast<ACrashWorldSettings>(GetWorldSettings()))
	{
		if (CrashWorldSettings->IntroCinematic)
		{
			FMovieSceneSequencePlaybackSettings IntroCinematicSettings = FMovieSceneSequencePlaybackSettings();
			IntroCinematicSettings.bAutoPlay = true;
			IntroCinematicSettings.bHidePlayer = true;
			IntroCinematicSettings.LoopCount.Value = -1;
			IntroCinematicSettings.bDisableMovementInput = true;

			ULevelSequencePlayer::CreateLevelSequencePlayer(this, CrashWorldSettings->IntroCinematic, IntroCinematicSettings, IntroCinematicSequenceActor);
		}
	}
}
