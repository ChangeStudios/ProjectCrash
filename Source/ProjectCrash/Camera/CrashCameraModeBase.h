// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Engine/World.h"
#include "CrashCameraModeBase.generated.h"

class UCrashCameraComponent;

/**
 * ECrashCameraModeBlendFunction
 *
 *  Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class ECrashCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target. Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target. Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates. Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)
};



/**
 * Camera view data used to blend between camera modes.
 */
struct FCrashCameraModeView
{
public:

	/** Default constructor. */
	FCrashCameraModeView();

	/** Blends this view into the given view with the specified weight (0-1). */
	void Blend(const FCrashCameraModeView& Other, float Weight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};



/**
 * A "camera mode" that can be used to represent a viewing configuration for a camera.
 *
 * This should be subclassed into different camera modes.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UCrashCameraModeBase : public UObject
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCameraModeBase();



	// Initialization.

public:

	/** Called continuously to update the camera's current view and to handle transitioning between camera modes. */
	void UpdateCameraMode(float DeltaTime);

	/** Called when this camera mode is activated via the camera stack. */
	virtual void OnActivation() {}

	/** Called when this camera mode is deactivated via the camera stack. */
	virtual void OnDeactivation() {}



	// Camera view.

protected:

	/**
	 * Returns this camera's desired root location, around which it will pivot depending on its distance.
	 *
	 * Default implementation, uses the target actor's root location. If the target actor is a pawn, uses their view
	 * location.
	 */
	virtual FVector GetPivotLocation() const;

	/**
	 * Returns this camera's desired root rotation.
	 * 
	 * Default implementation uses the target actor's world rotation. If the target actor is a pawn, uses their view
	 * rotation.
	 */
	virtual FRotator GetPivotRotation() const;

	/**
	 * Updates this camera mode's internal view.
	 *
	 * Default implementation simply uses the current pivot location and rotation.
	 */
	virtual void UpdateView(float DeltaTime);

// Internals.
protected:

	/** This camera mode's current view data. */
	FCrashCameraModeView View;

	/** Horizontal field of view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170.0", ClampMin = "5.0", ClampMax = "170.0", Units = "Degrees"))
	float FieldOfView;

	/** Minimum pitch (vertical axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", DisplayName = "Minimum View Pitch", Meta = (UIMin = "-90.0", UIMax = "FMath::Min(ViewPitchMax, 90.0f)", ClampMin = "-90.0", ClampMax = "FMath::Min(ViewPitchMax, 90.0f)", Units = "Degrees"))
	float ViewPitchMin;

	/** Maximum pitch (vertical axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", DisplayName = "Maximum View Pitch", Meta = (UIMin = "FMath::Max(ViewPitchMin, -90.0f)", UIMax = "90.0", ClampMin = "FMath::Max(ViewPitchMin, -90.0f)", ClampMax = "90.0", Units = "Degrees"))
	float ViewPitchMax;

	/** Minimum yaw (horizontal axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", DisplayName = "Minimum View Yaw", Meta = (UIMin = "0.0", UIMax = "FMath::Min(ViewPitchMax, 360.0f)", ClampMin = "0.0", ClampMax = "FMath::Min(ViewPitchMax, 360.0f)", Units = "Degrees"))
	float ViewYawMin;

	/** Maximum yaw (horizontal axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", DisplayName = "Maximum View Yaw", Meta = (UIMin = "FMath::Max(ViewPitchMin, 0.0f)", UIMax = "360.0", ClampMin = "FMath::Max(ViewPitchMin, 0.0f)", ClampMax = "360.0", Units = "Degrees"))
	float ViewYawMax;



	// Blending.

protected:

	/** Updates this camera mode's current blending values. */
	virtual void UpdateBlending(float DeltaTime);

// Internals.
protected:

	/** How long it takes to blend into this camera mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	/** The type of interpolation function used when blending into this camera mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	ECrashCameraModeBlendFunction BlendFunction;

	/** Exponent used by this camera mode's blend function to control the shape of the curve. */
	UPROPERTY(EditDefaultsOnly, Category = "Blending", Meta = (ClampMin = "0.0"))
	float BlendExponent;

	/** Linear blend alpha used for interpolation. */
	float BlendAlpha;

	/** Blend weight calculated using the blend alpha and blend function. */
	float BlendWeight;



	// Utils.

public:
	
	/** Returns this camera mode's outer object, which should always be a camera component. */
	UCrashCameraComponent* GetCrashCameraComponent() const;

	/** Retrieves the world through the outer object. */
	virtual UWorld* GetWorld() const override;

	/** Helper for retrieving the outer camera component's target actor. */
	AActor* GetTargetActor() const;
};


UCLASS()
class UCrashCameraModeStack : public UObject
{
	GENERATED_BODY()

};