// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashCameraMode_StaticCameraBase.h"

#include "CrashCameraComponent.h"

UCrashCameraMode_StaticCameraBase::UCrashCameraMode_StaticCameraBase() :
	bCanTurn(true)
{
}

void UCrashCameraMode_StaticCameraBase::OnActivation()
{
	Super::OnActivation();

	// Stop controlling the pawn's rotation with the camera.
	if (APawn* OwningPawn = Cast<APawn>(GetCrashCameraComponent()->GetOwner()))
	{
		OwningPawn->bUseControllerRotationYaw = false;
		OwningPawn->bUseControllerRotationPitch = false;
		OwningPawn->bUseControllerRotationRoll = false;
	}
}

void UCrashCameraMode_StaticCameraBase::OnDeactivation()
{
	// Start controlling the pawn's rotation with the camera.
	if (APawn* OwningPawn = Cast<APawn>(GetCrashCameraComponent()->GetOwner()))
	{
		OwningPawn->bUseControllerRotationYaw = true;
		OwningPawn->bUseControllerRotationPitch = true;
		OwningPawn->bUseControllerRotationRoll = true;
	}

	Super::OnDeactivation();
}

FVector UCrashCameraMode_StaticCameraBase::GetPivotLocation() const
{
	return CameraLocation;
}

FRotator UCrashCameraMode_StaticCameraBase::GetPivotRotation() const
{
	// If the viewer is allowed to turn this camera, use their controller rotation.
	if (bCanTurn)
	{
		return Super::GetPivotRotation();
	}
	// Use the camera's fixed rotation if the viewer is not allowed to turn on their own.
	else
	{
		return CameraRotation;
	}
}
