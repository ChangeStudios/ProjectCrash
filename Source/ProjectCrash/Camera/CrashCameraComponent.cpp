// Copyright Samuel Reitich. All rights reserved.


#include "CrashCameraComponent.h"

#include "CrashCameraModeBase.h"
#include "Engine/Canvas.h"


UCrashCameraComponent::UCrashCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
}

void UCrashCameraComponent::OnRegister()
{
	Super::OnRegister();

	// Create this component's camera stack, if it doesn't have one yet.
	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UCrashCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UCrashCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	// Check if we need to push any new camera modes to the stack.
	UpdateCameraModes();

	// Update the camera modes on the stack.
	FCrashCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);


	/* Keep player controllers' view rotation in sync with this camera's view, if this camera component is owned by a
	 * pawn with a PC. */
	if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = OwningPawn->GetController<APlayerController>())
		{
			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}


	// Keep the camera component's view in sync with the current camera mode.
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;


	// Fill in the desired view.
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;


	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}
}

void UCrashCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	// Check if any new camera modes should be pushed to the stack.
	if (CameraModeStack->IsStackActivated())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UCrashCameraModeBase> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}

void UCrashCameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("CrashCameraComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("	Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("	Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("	FOV: %f"), FieldOfView));

	// Draw debug information for the current camera mode stack.
	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}
