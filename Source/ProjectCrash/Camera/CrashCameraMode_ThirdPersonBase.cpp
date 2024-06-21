// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashCameraMode_ThirdPersonBase.h"

#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"

UCrashCameraMode_ThirdPersonBase::UCrashCameraMode_ThirdPersonBase() :
	TargetOffsetCurve(nullptr),
	PenetrationBlendInTime(0.1f),
	PenetrationBlendOutTime(0.15f),
	bPreventPenetration(true),
	bDoPredictiveAvoidance(true),
	CollisionPushOutDistance(2.0f),
	AimLineToDesiredPosBlockedPct(0.0f)
{
	// Base penetration avoidance feeler.
	PenetrationPreventionFeelers.Add(FPenetrationPreventionFeeler(FRotator(+00.0f, +00.0f, +0.0f), 0.50f, 1.00f, 5.00f, 0));

	// Predictive avoidance feelers.
	PenetrationPreventionFeelers.Add(FPenetrationPreventionFeeler(FRotator(+00.0f, +00.0f, +5.0f), 0.25f, 0.50f, 5.00f, 3));
	PenetrationPreventionFeelers.Add(FPenetrationPreventionFeeler(FRotator(+00.0f, +00.0f, -5.0f), 0.25f, 0.50f, 5.00f, 3));
	PenetrationPreventionFeelers.Add(FPenetrationPreventionFeeler(FRotator(+00.0f, +15.0f, +0.0f), 0.25f, 0.50f, 5.00f, 3));
	PenetrationPreventionFeelers.Add(FPenetrationPreventionFeeler(FRotator(+00.0f, -15.0f, +0.0f), 0.25f, 0.50f, 5.00f, 3));
}

void UCrashCameraMode_ThirdPersonBase::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotationClamped();

	// Use the current pivot location and rotation as the camera's root view.
	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;

	// Apply third-person offset as a function of the camera's current pitch.
	if (TargetOffsetCurve)
	{
		const FVector TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
		View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
	}

	// Adjust the final position for penetration avoidance.
	UpdatePreventPenetration(DeltaTime);
}

void UCrashCameraMode_ThirdPersonBase::UpdatePreventPenetration(float DeltaTime)
{
	if (!bPreventPenetration)
	{
		return;
	}

	// Use the target actor's root primitive component as the penetration prevention target.
	AActor* TargetActor = GetTargetActor();
	if (UPrimitiveComponent* TargetActorRoot = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
	{
		// Find the closest point on our camera ray (our aim) to the target actor.
		FVector ClosestPointFromAimToTarget;
		FVector SafeLocation = TargetActor->GetActorLocation();
		FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointFromAimToTarget);

		// Adjust the safe distance height to match our aim.
		float const PushInDistance = PenetrationPreventionFeelers[0].Extent + CollisionPushOutDistance;
		float const MaxHalfHeight = TargetActor->GetSimpleCollisionHalfHeight() - PushInDistance;
		SafeLocation.Z = FMath::Clamp(ClosestPointFromAimToTarget.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

		// Get the distance from our aim point to the target's collision.
		float DistanceSqr;
		TargetActorRoot->GetSquaredDistanceToCollision(ClosestPointFromAimToTarget, DistanceSqr, SafeLocation);

		// Push back inside the capsule to avoid penetration while doing our line traces.
		if (PenetrationPreventionFeelers.Num() > 0)
		{
			SafeLocation += (SafeLocation - ClosestPointFromAimToTarget).GetSafeNormal() * PushInDistance;
		}

		// Perform camera penetration prevention calculations.
		bool const bSingleRayPenetrationCheckOnly = !bDoPredictiveAvoidance;
		PreventCameraPenetration(*TargetActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheckOnly);
	}
}

void UCrashCameraMode_ThirdPersonBase::PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	// Reset penetration prevention debug.
	DebugActorsHitDuringCameraPenetration.Reset();
#endif // ENABLE_DRAW_DEBUG

	// Blocking percent for the base ray.
	float HardBlockedPct = DistBlockedPct;
	// Blocking percent for predictive rays.
	float SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;

	// Get the forward, right, and up axes of the base ray.
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	float DistBlockedPctThisFrame = 1.0f;

	// Construct params for the penetration prevention feeler traces.
	int32 const NumRaysToTrace = bSingleRayOnly ? FMath::Min(1, PenetrationPreventionFeelers.Num()) : PenetrationPreventionFeelers.Num();
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr);
	SphereParams.AddIgnoredActor(&ViewTarget);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.0f);
	UWorld* World = GetWorld();

	// Perform penetration prevention feeler traces.
	for (int32 RayIndex = 0; RayIndex < NumRaysToTrace; ++RayIndex)
	{
		FPenetrationPreventionFeeler& Feeler = PenetrationPreventionFeelers[RayIndex];
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			// Calculate the target ray for this feeler.
			FVector RayTarget;
			FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
			RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
			RayTarget = SafeLoc + RotatedRay;

			// Cast for world and pawn hits separately to safely ignore the camera's target pawn.
			SphereShape.Sphere.Radius = Feeler.Extent;
			ECollisionChannel TraceChannel = ECC_Camera;

			// Perform the trace.
			FHitResult Hit;
			const bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams);

			// Debug trace if desired.
#if ENABLE_DRAW_DEBUG
			if (World->TimeSince(LastDrawDebugTime) < 1.0f)
			{
				// Safe location.
				DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, FColor::Red);
				// Trace hit/ending location.
				DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, SphereShape.Sphere.Radius, 8, FColor::Red);
				// Trace.
				DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
			}
#endif // ENABLE_DRAW_DEBUG

			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

			const AActor* HitActor = Hit.GetActor();

			if (bHit && HitActor)
			{
				bool bIgnoreHit = false;

				// Ignore CameraBlockingVolume hits that occur in front of the target.
				if (HitActor->IsA<ACameraBlockingVolume>())
				{
					// Check if the blocking volume is in FRONT of the target.
					const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
					const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
					const FVector HitOffset = Hit.Location - ViewTargetLocation;
					const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
					const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);

					// Ignore the volume if it's in front of the target.
					if (DotHitDirection > 0.0f)
					{
						bIgnoreHit = true;

						// Ignore this CameraBlockingVolume for the remaining traces.
						SphereParams.AddIgnoredActor(HitActor);
					}
					else
					{
#if ENABLE_DRAW_DEBUG
						DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif // ENABLE_DRAW_DEBUG
					}
				}

				if (!bIgnoreHit)
				{
					// Calculate the blocking percent of the hit.
					float const Weight = Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight;
					float NewBlockPct = Hit.Time;
					NewBlockPct += (1.0f - NewBlockPct) * (1.0f - Weight);

					// Recompute blocked percent, taking into account push-out distance.
					NewBlockPct = ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();
					DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

					// Always perform traces for feelers that are hitting something.
					Feeler.FramesUntilNextTrace = 0;

#if ENABLE_DRAW_DEBUG
					DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif // ENABLE_DRAW_DEBUG
				}
			}

			if (RayIndex == 0)
			{
				// Don't interpolate to the base ray; snap to it.
				HardBlockedPct = DistBlockedPctThisFrame;
			}
			else
			{
				// Interpolate predictive rays.
				SoftBlockedPct = DistBlockedPctThisFrame;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	// Skip interpolation if desired.
	if (bResetInterpolation)
	{
		DistBlockedPct = DistBlockedPctThisFrame;
	}
	// Interpolate out of safe location when penetration preventions ends.
	else if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	// Interpolate into safe location when penetration prevention is active.
	else
	{
		if (DistBlockedPct > SoftBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	// Final camera offset.
	DistBlockedPct = FMath::Clamp(DistBlockedPct, 0.0f, 1.0f);
	if (DistBlockedPct < (1.0f - ZERO_ANIMWEIGHT_THRESH))
	{
		CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
	}
}

void UCrashCameraMode_ThirdPersonBase::DrawDebug(UCanvas* Canvas) const
{
	Super::DrawDebug(Canvas);

#if ENABLE_DRAW_DEBUG
	// Display the name of any actors hit during camera penetration prevention.
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	for (int i = 0; i < DebugActorsHitDuringCameraPenetration.Num(); i++)
	{
		DisplayDebugManager.DrawString(
			FString::Printf(TEXT("Hit Actor During Penetration Prevention [%d]: %s"), i, *DebugActorsHitDuringCameraPenetration[i]->GetName()));
	}

	LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif // ENABLE_DRAW_DEBUG
}
