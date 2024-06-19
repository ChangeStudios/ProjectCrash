// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashCameraModeBase.h"

#include "CrashCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

/**
 * FCameraModeView
 */
FCrashCameraModeView::FCrashCameraModeView() :
    Location(ForceInit),
	Rotation(ForceInit),
	ControlRotation(ForceInit),
	FieldOfView(100.0f) // TODO: Change to preprocessor var
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
	FieldOfView(100.0f), // TODO: Change to preprocessor var
	ViewPitchMin(-90.0f),
	ViewPitchMax(90.0f),
	ViewYawMin(0.0f),
	ViewYawMax(360.0f),

	BlendTime(0.25f),
	BlendFunction(ECrashCameraModeBlendFunction::EaseOut),
	BlendExponent(4.0f),
	BlendAlpha(1.0f),
	BlendWeight(1.0f)
{
}

void UCrashCameraModeBase::UpdateCameraMode(float DeltaTime)
{
	// Update our current view.
	UpdateView(DeltaTime);

	// Progress any ongoing interpolation
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

void UCrashCameraModeBase::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	// Apply view angle clamps.
	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);
	PivotRotation.Yaw = FMath::ClampAngle(PivotRotation.Yaw, ViewYawMin, ViewYawMax);

	// Use the current pivot location and rotation as the camera's view for this default implementation.
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
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
