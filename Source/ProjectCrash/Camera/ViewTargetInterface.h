// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ViewTargetInterface.generated.h"

class UCrashCameraModeBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UViewTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implements camera mode events. Implement this on actors that can be camera view targets to respond to camera mode
 * events.
 *
 * For example, this interface is used by CrashCharacter to switch between first-person and third-person meshes when a
 * first-person camera mode is activated or deactivated.
 */
class PROJECTCRASH_API IViewTargetInterface
{
	GENERATED_BODY()

public:

	/** Called when a camera mode of which this actor is the view target begins blending in. I.e. its weight is greater
	 * than 0.0. */
	virtual void OnStartCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode) {}

	/** Called when a camera mode of which this actor is the view target finishes blending in. I.e. its weight is
	 * 1.0. */
	virtual void OnFinishCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode) {}
};
