// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashCameraMode_ThirdPersonBase.h"

UCrashCameraMode_ThirdPersonBase::UCrashCameraMode_ThirdPersonBase() :
	TargetOffsetCurve(nullptr),
	PenetrationBlendInTime(0.1f),
	PenetrationBlendOutTime(0.15f),
	bPreventPenetration(true),
	bDoPredictiveAvoidance(true),
	CollisionPushOutDistance(2.0f),
	ReportPenetrationPercent(0.0f),
	AimLineToDesiredPosBlockedPct(0.0f)
{
	// Base penetration avoidance feeler.
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));

	// Predictive avoidance feelers.
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));
}

void UCrashCameraMode_ThirdPersonBase::UpdateView(float DeltaTime)
{
	Super::UpdateView(DeltaTime);
}

void UCrashCameraMode_ThirdPersonBase::UpdateForTarget(float DeltaTime)
{
}

void UCrashCameraMode_ThirdPersonBase::UpdatePreventPenetration(float DeltaTime)
{
}

void UCrashCameraMode_ThirdPersonBase::PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
}

void UCrashCameraMode_ThirdPersonBase::DrawDebug(UCanvas* Canvas) const
{
	Super::DrawDebug(Canvas);
}
