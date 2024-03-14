// Copyright Samuel Reitich 2024.


#include "Player/PlayerControllers/MenuPlayerController.h"

#include "EngineUtils.h"
#include "Camera/CameraActor.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Enable only UI input.
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true); // TODO: Change this to be platform-specific

	// Find and possess the first camera actor in the world.
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		ACameraActor* Camera = *It;
		FViewTargetTransitionParams CamTransition = FViewTargetTransitionParams();
		CamTransition.BlendTime = 0.0f;
		CamTransition.bLockOutgoing = false;
		SetViewTarget(Camera, CamTransition);
		break;
	}
}
