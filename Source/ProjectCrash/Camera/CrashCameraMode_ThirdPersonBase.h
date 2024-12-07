// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CrashCameraModeBase.h"
#include "CrashCameraMode_ThirdPersonBase.generated.h"

class UCurveVector;

/**
 * Defines a ray used to detect and predict camera collisions to avoid penetration.
 */
USTRUCT()
struct FPenetrationPreventionFeeler
{
	GENERATED_BODY()

	/** Deviance from the base ray (i.e. the camera's "arm"). */
	UPROPERTY(EditAnywhere, Category = "Penetration Avoidance Feeler", DisplayName = "Adjustment Rotation")
	FRotator AdjustmentRot;

	/** How much this feeler influences the final camera position, if it hits world geometry. */
	UPROPERTY(EditAnywhere, Category = "Penetration Avoidance Feeler")
	float WorldWeight;

	/** How much this feeler influences the final camera position, if it hits a pawn. */
	UPROPERTY(EditAnywhere, Category = "Penetration Avoidance Feeler")
	float PawnWeight;

	/** The radius of this feeler's tracing ray. */
	UPROPERTY(EditAnywhere, Category = "Penetration Avoidance Feeler")
	float Extent;

	/** Minimum frame interval between traces with this feeler, if nothing was hit last frame. 0 is every frame. */
	UPROPERTY(EditAnywhere, Category = "Penetration Avoidance Feeler", DisplayName = "Adjustment Rotation")
	int32 TraceInterval;

	/** Number of frames before this feeler should be traced again. */
	UPROPERTY(Transient)
	int32 FramesUntilNextTrace;



	/** Default constructor. */
	FPenetrationPreventionFeeler() :
		AdjustmentRot(FRotator::ZeroRotator),
		WorldWeight(0),
		PawnWeight(0),
		Extent(0),
		TraceInterval(0),
		FramesUntilNextTrace(0)
	{}

	/** Parameterized constructor. */
	FPenetrationPreventionFeeler(const FRotator& InAdjustmentRot,
									const float& InWorldWeight, 
									const float& InPawnWeight, 
									const float& InExtent, 
									const int32& InTraceInterval = 0, 
									const int32& InFramesUntilNextTrace = 0) :
		AdjustmentRot(InAdjustmentRot),
		WorldWeight(InWorldWeight),
		PawnWeight(InPawnWeight),
		Extent(InExtent),
		TraceInterval(InTraceInterval),
		FramesUntilNextTrace(InFramesUntilNextTrace)
	{}
};



/**
 * Base third-person camera. Extends the base camera mode with positional offsets and penetration avoidance.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UCrashCameraMode_ThirdPersonBase : public UCrashCameraModeBase
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCameraMode_ThirdPersonBase();



	// Camera view.

protected:

	/** Offsets this camera mode's view using its target offset curve. Applies penetration prevention if desired. */
	virtual void UpdateView(float DeltaTime) override;

	/** Defines the camera's location offset as a function of the camera's rotation. */
	UPROPERTY(EditDefaultsOnly, Category = "Third Person Camera")
	TObjectPtr<const UCurveVector> TargetOffsetCurve;



	// Penetration prevention.

protected:

	/** Applies penetration prevention to the camera's current view. */
	void UpdatePreventPenetration(float DeltaTime);

	/** Helper function for performing penetration prevention calculations. */
	void PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

// Penetration prevention properties.
protected:

	/** Amount of time over which the camera will interpolate to a safe location while avoiding penetration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration Prevention")
	float PenetrationBlendInTime;

	/** Amount of time over which the camera will interpolate from its safe location back to the default position when
	 * penetration is no longer being prevented. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration Prevention")
	float PenetrationBlendOutTime;

	/** Whether to perform collision checks to prevent the camera from phasing into geometry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration Prevention")
	bool bPreventPenetration;

	/** Whether to detect nearby geometry and move the camera in anticipation, rather than waiting until the camera
	 * actually hits something. Helps prevent sudden camera pops. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Penetration Prevention")
	bool bDoPredictiveAvoidance;

	/** Distance from geometry we want to keep the camera when preventing penetration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Penetration Prevention")
	float CollisionPushOutDistance;

	/**
	 * Rays used to detect camera collisions and prevent penetration.
	 * Index: 0  : Default feeler we use to prevent collisions.
	 * Index: 1+ : Used if bDoPredictiveAvoidance is true, to scan for potential impacts if the player were to rotate
	 *             towards that direction and collide the camera, so that it pulls in before an impact occurs.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Penetration Prevention")
	TArray<FPenetrationPreventionFeeler> PenetrationPreventionFeelers;

// Internals.
public:

	/** The percent from the base aim line (our default view) to the safe location while avoiding penetration. Used for
	 * interpolating to and from the safe location to avoid penetration. */
	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	/** Actors hit while performing penetration prevention. Used for debugging. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;



	// Utils.

public:

	/** Draws debugging information for the actors that were hit while preventing camera penetration. */
	virtual void DrawDebug(UCanvas* Canvas) const override;

protected:

#if ENABLE_DRAW_DEBUG
	/** Tracks when we last drew debugging for camera penetration prevention. */
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif
};
