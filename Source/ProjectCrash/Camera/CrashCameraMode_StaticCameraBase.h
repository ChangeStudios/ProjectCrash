// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CrashCameraModeBase.h"
#include "CrashCameraMode_StaticCameraBase.generated.h"

/**
 * A camera detached from the camera component's owning actor, whose view pivots around a preset transform.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UCrashCameraMode_StaticCameraBase : public UCrashCameraModeBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCameraMode_StaticCameraBase();



	// Initialization.

public:

	/** Stops this camera's owning pawn from rotating with its camera, so this camera's view does not control its pawn's
	 * rotation. */
	// TODO: Make this a camera type tag? E.g. "ControlsPawnRotation"?
	virtual void OnActivation() override;

	/** Re-enables this camera's owning pawn rotating with its camera, so the camera can start controlling the pawn's
	 * rotation again. */
	virtual void OnDeactivation() override;



	// Camera view.

protected:

	/** Uses this camera mode's fixed location as the desired camera location. */
	virtual FVector GetPivotLocation() const override;

	/** Uses this camera mode's fixed rotation as the desired camera rotation. Uses the viewer's controller rotation if
	 * CanTurn is true. */
	virtual FRotator GetPivotRotation() const override;



	// Camera properties.

protected:

	/** The location at which this camera will be placed. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Properties")
	FVector CameraLocation;

	/** If true, the player can turn the camera using their controller rotation. If false, the camera's rotation is
	 * fixed. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Properties")
	bool bCanTurn;

	/** The fixed camera rotation to use if CanTurn is true. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Properties", Meta = (EditCondition = "!bCanTurn"))
	FRotator CameraRotation;
};
