// Copyright Samuel Reitich. All rights reserved.


#include "Player/CrashPlayerController.h"

#include "Camera/CrashPlayerCameraManager.h"

ACrashPlayerController::ACrashPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ACrashPlayerCameraManager::StaticClass();
}
