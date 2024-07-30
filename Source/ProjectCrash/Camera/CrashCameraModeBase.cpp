// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashCameraModeBase.h"

#include "CrashCameraComponent.h"
#include "CrashPlayerCameraManager.h"
#include "ViewTargetInterface.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "GameFramework/CrashLogging.h"

/**
 * FCameraModeView
 */
FCrashCameraModeView::FCrashCameraModeView() :
    Location(ForceInit),
	Rotation(ForceInit),
	ControlRotation(ForceInit),
	FieldOfView(CAMERA_DEFAULT_FOV)
{
}

void FCrashCameraModeView::Blend(const FCrashCameraModeView& Other, float Weight)
{
	// Blend does not affect this view.
	if (Weight <= 0.0f)
	{
		return;
	}

	// Blend fully to the other view.
	if (Weight >= 1.0f)
	{
		*this = Other;
		return;
	}

	// Blend location.
	Location = FMath::Lerp(Location, Other.Location, Weight);

	// Blend rotation.
	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (Weight * DeltaRotation);

	// Blend control rotation.
	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (Weight * DeltaControlRotation);

	// Blend FOV.
	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, Weight);
}



/**
 * UCrashCameraModeBase
 */
UCrashCameraModeBase::UCrashCameraModeBase() :
	FieldOfView(CAMERA_DEFAULT_FOV),
	ViewPitchMin(-90.0f),
	ViewPitchMax(90.0f),
	ViewYawMin(0.0f),
	ViewYawMax(359.9f),

	BlendTime(0.25f),
	BlendFunction(ECrashCameraModeBlendFunction::EaseOut),
	BlendExponent(4.0f),
	BlendAlpha(1.0f),
	BlendWeight(1.0f),
	bResetInterpolation(0)
{
}

void UCrashCameraModeBase::UpdateCameraMode(float DeltaTime)
{
	// Update our current view.
	UpdateView(DeltaTime);

	// Progress any ongoing blending
	UpdateBlending(DeltaTime);
}

FVector UCrashCameraModeBase::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	/* Use characters' capsule components to determine their view. This accounts for runtime changes to view location,
	 * like crouching. */
	if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
		check(TargetCharacterCDO);

		const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
		check(CapsuleComp);

		const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
		check(CapsuleCompCDO);

		const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
		const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;

		return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
	}

	// Use pawns' view location.
	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetPawnViewLocation();
	}

	// Use non-pawn actors' root location.
	return TargetActor->GetActorLocation();
}

FRotator UCrashCameraModeBase::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	// Use pawns' view rotation.
	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	// Use non-pawn actors' root rotation.
	return TargetActor->GetActorRotation();
}

FRotator UCrashCameraModeBase::GetPivotRotationClamped() const
{
	// Get the base pivot rotation.
	FRotator PivotRotation = GetPivotRotation();

	// Apply view angle clamps.
	if (bClampPitch)
	{
		PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);
	}
	if (bClampYaw)
	{
		PivotRotation.Yaw = FMath::ClampAngle(PivotRotation.Yaw, ViewYawMin, ViewYawMax);
	}

	return PivotRotation;
}

void UCrashCameraModeBase::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotationClamped();

	// Use the current pivot location and rotation as the camera's view for this default implementation.
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UCrashCameraModeBase::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Clamp exponent above 0.0. Default to 1.0.
	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;
	const float InvExponent = 1.0f / Exponent;

	/* Recalculate the blend alpha. We normally calculate the weight from the alpha. But since we're setting the weight
	 * directly, we need to use an inverse exponent to calculate the alpha. */
	switch (BlendFunction)
	{
		case ECrashCameraModeBlendFunction::Linear:
			BlendAlpha = BlendWeight;
			break;

		case ECrashCameraModeBlendFunction::EaseIn:
			BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		case ECrashCameraModeBlendFunction::EaseOut:
			BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		case ECrashCameraModeBlendFunction::EaseInOut:
			BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
			break;

		default:
			checkf(false, TEXT("Invalid blend function [%d] used for camera mode [%s]."), (uint8)BlendFunction, *GetNameSafe(this));
			break;
	}
}

void UCrashCameraModeBase::UpdateBlending(float DeltaTime)
{
	// Calculate current blend alpha.
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	// Clamp exponent above 0.0. Default to 1.0.
	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	// Calculate the blend weight depending on the blend function.
	switch (BlendFunction)
	{
		case ECrashCameraModeBlendFunction::Linear:
			BlendWeight = BlendAlpha;
			break;

		case ECrashCameraModeBlendFunction::EaseIn:
			BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		case ECrashCameraModeBlendFunction::EaseOut:
			BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		case ECrashCameraModeBlendFunction::EaseInOut:
			BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
			break;

		default:
			checkf(false, TEXT("Invalid blend function [%d] used for camera mode [%s]."), (uint8)BlendFunction, *GetNameSafe(this));
			break;
	}
}

UCrashCameraComponent* UCrashCameraModeBase::GetCrashCameraComponent() const
{
	// Camera modes' outer objects should always be a CrashCameraComponent.
	return CastChecked<UCrashCameraComponent>(GetOuter());
}

UWorld* UCrashCameraModeBase::GetWorld() const
{
	// Check for CDO.
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UCrashCameraModeBase::GetTargetActor() const
{
	// Use the outer camera's target actor.
	return GetCrashCameraComponent()->GetTargetActor();
}

void UCrashCameraModeBase::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	// Display this camera mode's name and current weight.
	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("			Camera Mode: %s (Weight: %f, Type: %s)"), *GetName(), BlendWeight, *CameraTypeTag.ToString()));
}

#if WITH_EDITOR
void UCrashCameraModeBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedProperty = PropertyChangedEvent.GetMemberPropertyName();

	// Clamp minimum pitch below maximum pitch.
	if (ChangedProperty == GET_MEMBER_NAME_CHECKED(UCrashCameraModeBase, ViewPitchMin))
	{
		if (ViewPitchMin > ViewPitchMax)
		{
			ViewPitchMin = ViewPitchMax;
		}
	}

	// Clamp maximum pitch above minimum pitch.
	else if (ChangedProperty == GET_MEMBER_NAME_CHECKED(UCrashCameraModeBase, ViewPitchMax))
	{
		if (ViewPitchMax < ViewPitchMin)
		{
			ViewPitchMax = ViewPitchMin;
		}
	}

	// Clamp minimum yaw below maximum yaw.
	else if (ChangedProperty == GET_MEMBER_NAME_CHECKED(UCrashCameraModeBase, ViewYawMin))
	{
		if (ViewYawMin > ViewYawMax)
		{
			ViewYawMin = ViewYawMax;
		}
	}

	// Clamp maximum yaw above minimum yaw.
	else if (ChangedProperty == GET_MEMBER_NAME_CHECKED(UCrashCameraModeBase, ViewYawMax))
	{
		if (ViewYawMax < ViewYawMin)
		{
			ViewYawMax = ViewYawMin;
		}
	}
}
#endif // WITH_EDITOR



/**
 * UCrashCameraModeStack
 */
UCrashCameraModeStack::UCrashCameraModeStack()
{
	// Stacks start as active. There should be no camera modes at this point.
	bIsActive = true;
}

void UCrashCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;

		// Notify all camera modes on the stack of activation.
		for (UCrashCameraModeBase* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UCrashCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;

		// Notify all camera modes on the stack of deactivation.
		for (UCrashCameraModeBase* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

void UCrashCameraModeStack::PushCameraMode(TSubclassOf<UCrashCameraModeBase> CameraModeClass)
{
	if (!ensure(CameraModeClass))
	{
		return;
	}

	// Create a new camera mode instance of the specified type, or retrieve the instance if it already exists.
	UCrashCameraModeBase* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);


	// If the new camera mode is already at the top of the stack, we don't have to do anything else.
	int32 StackSize = CameraModeStack.Num();
	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
	{
		return;
	}


	// Check if the new camera mode is already in the camera stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; StackIndex++)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution = CameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	// If the camera mode is already in the stack, remove its existing instance. Keep the new one.
	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}


	/* Determine the weight with which to start blending the new camera mode. The camera mode should only blend if it
	 * has a blend time set and there's an existing camera mode from which to blend (i.e. this isn't the first camera in
	 * the stack). If the camera mode was already in the stack, start blending from where that existing camera was. */ 
	const bool bShouldBlend = (CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0);
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);

	// Add the camera mode to the top of the stack.
	CameraModeStack.Insert(CameraMode, 0);

	// Notify the new camera's target that it is blending in.
	if (AActor* TargetActor = CameraMode->GetTargetActor())
	{
		if (IViewTargetInterface* ViewTargetInterface = Cast<IViewTargetInterface>(TargetActor))
		{
			UCrashCameraModeBase* PreviousCameraMode = ((CameraModeStack.Num() > 1) ? CameraModeStack[1] : nullptr);

			if (CameraMode != PreviousCameraMode)
			{
				ViewTargetInterface->OnStartCameraModeBlendIn(PreviousCameraMode, CameraMode);

				// If this is the first camera mode, then it skips blending in, and can immediately finish.
				if (!bShouldBlend)
				{
					ViewTargetInterface->OnFinishCameraModeBlendIn(PreviousCameraMode, CameraMode);
				}
			}
		}
	}

	/* Make sure the bottom of the stack is always weight to 1.0. If the new camera mode is the first camera in the
	 * stack, this causes it to skip blending, and immediately cut to it. */
	// NOTE: This is disabled because we currently track the bottom camera's weight in BlendStack.
	// CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Notify the camera mode that it was added to the stack, if it wasn't already in it.
	if (ExistingStackIndex == INDEX_NONE)
	{
		CameraMode->OnActivation();
	}
}	

bool UCrashCameraModeStack::EvaluateStack(float DeltaTime, FCrashCameraModeView& OutCameraModeView)
{
	// Don't evaluate inactive camera stacks.
	if (!bIsActive)
	{
		return false;
	}

	// Re-evaluate the current camera mode.
	UpdateStack(DeltaTime);

	// Progress any ongoing blending.
	BlendStack(OutCameraModeView);

	return true;
}

void UCrashCameraModeStack::GetBlendInfo(float& OutTopCameraWeight, FGameplayTag& OutTopCameraTag) const
{
	// Make sure there is at least one active camera mode.
	if (CameraModeStack.Num())
	{
		UCrashCameraModeBase* TopCamera = CameraModeStack.Last();
		check(TopCamera)
		OutTopCameraWeight = TopCamera->GetBlendWeight();
		OutTopCameraTag = TopCamera->GetCameraTypeTag();
	}
	// Generic data for active camera modes.
	else
	{
		OutTopCameraWeight = 1.0f;
		OutTopCameraTag = FGameplayTag();
	}
}

float UCrashCameraModeStack::GetCameraWeightByTag(FGameplayTag CameraTag) const
{
	for (UCrashCameraModeBase* CameraMode : CameraModeStack)
	{
		if (CameraMode->GetCameraTypeTag() == CameraTag)
		{
			return CameraMode->GetBlendWeight();
		}
	}

	// No camera with the given tag was found.
	return 0.0f;
}

void UCrashCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("	--- Camera Modes (Begin) ---")));

	// Draw debug information for each camera mode in this stack.
	for (const UCrashCameraModeBase* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("	--- Camera Modes (End) ---")));
}

UCrashCameraModeBase* UCrashCameraModeStack::GetCameraModeInstance(TSubclassOf<UCrashCameraModeBase> CameraModeClass)
{
	check(CameraModeClass);

	// Search for an existing camera mode instance.
	for (UCrashCameraModeBase* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// If a camera mode instance of the specified type does not exist yet, create one.
	UCrashCameraModeBase* NewCameraMode = NewObject<UCrashCameraModeBase>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	// Cache the new camera mode instance.
	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UCrashCameraModeStack::UpdateStack(float DeltaTime)
{
	// No camera modes.
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}


	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	// Update each camera mode.
	for (int32 StackIndex = 0; StackIndex < StackSize; StackIndex++)
	{
		UCrashCameraModeBase* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);
		const float PreviousWeight = CameraMode->GetBlendWeight();

		CameraMode->UpdateCameraMode(DeltaTime);

		/* If we have a camera mode with a blend weight of 1.0, that should be our currently viewed camera mode.
		 * Everything below it is irrelevant and can be removed. */
		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);

			// If we've just finished blending in, notify the camera's target.  
			if (PreviousWeight < 1.0f)
			{
				if (AActor* TargetActor = CameraMode->GetTargetActor())
				{
					if (IViewTargetInterface* ViewTargetInterface = Cast<IViewTargetInterface>(TargetActor))
					{
						UCrashCameraModeBase* PreviousCameraMode = StackSize > 0 ? PreviousCameraMode = CameraModeStack[1] : nullptr;

						if (CameraMode != PreviousCameraMode)
						{
							ViewTargetInterface->OnFinishCameraModeBlendIn(PreviousCameraMode, CameraMode);
						}
					}
				}
			}

			break;
		}
	}


	// Remove any camera modes that are now irrelevant.
	if (RemoveCount > 0)
	{
		// Notify the camera modes of deactivation.
		for (int32 StackIndex = RemoveIndex; StackIndex < RemoveIndex; StackIndex++)
		{
			UCrashCameraModeBase* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);

			CameraMode->OnDeactivation();
		}

		// Remove the camera mode from the stack.
		CameraModeStack.RemoveAt(RemoveIndex);
	}
}

void UCrashCameraModeStack::BlendStack(FCrashCameraModeView& OutCameraModeView) const
{
	// No camera modes.
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	// Start with the bottom camera mode.
	UCrashCameraModeBase* BaseCameraMode = CameraModeStack[StackSize - 1];
	check(BaseCameraMode);
	float NewBaseCameraWeight = 1.0f;

	const UCrashCameraModeBase* CameraMode = BaseCameraMode;
	OutCameraModeView = CameraMode->GetCameraModeView();

	// Take our bottom camera's view and interpolate it towards each weighted camera's view.
	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);
		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());

		// Calculate how much weight we're losing from our base view.
		NewBaseCameraWeight *= (1.0f - CameraMode->GetBlendWeight());
	}

	// Update our base view's weight with however much we've lost to other cameras blending in.
	BaseCameraMode->SetBlendWeight(NewBaseCameraWeight);
}
