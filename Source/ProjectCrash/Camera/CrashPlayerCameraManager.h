// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "GameplayTagContainer.h"
#include "CrashPlayerCameraManager.generated.h"

#define CAMERA_DEFAULT_FOV			(90.0f)

/**
 * Default player camera manager for this project.
 */
UCLASS(NotPlaceable, MinimalAPI, Meta = (BlueprintSpawnableComponent = "false"))
class ACrashPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	/** Gets the current camera component's top-most camera mode's weight and identifying tag. Useful for determining
	 * the local player's current camera type, e.g. "First-Person." */
	void GetCameraBlendInfo(float& OutTopCameraWeight, FGameplayTag& OutTopCameraTag) const;

protected:

	/** Hooks the CrashCameraComponent into the camera debug system. */
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
};
