// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "CrashPlayerCameraManager.generated.h"

#define CAMERA_DEFAULT_FOV			(90.0f)

/**
 * Default player camera manager for this project.
 */
UCLASS(NotPlaceable, MinimalAPI)
class ACrashPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

protected:

	/** Hooks the CrashCameraComponent into the camera debug system. */
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
};
