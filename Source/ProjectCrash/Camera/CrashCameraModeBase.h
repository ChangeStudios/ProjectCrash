// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "CrashCameraModeBase.generated.h"

class UCrashCameraComponent;

/**
 * Blend function used for transitioning between camera modes.
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

	/** Continuously called to update the camera's current view and to handle transitioning between camera modes. */
	void UpdateCameraMode(float DeltaTime);

	/**
	 * Called when this camera mode is activated via the camera stack.
	 *
	 * This is only called when a new camera mode is activated. If a camera mode is added to a stack that already had
	 * an instance of that camera mode, this function is not called.
	 */
	virtual void OnActivation() {}

	/** Called when this camera mode is deactivated via the camera stack. */
	virtual void OnDeactivation() {}



	// Camera view.

public:

	/** Retrieves this camera mode's current view. */
	FORCEINLINE const FCrashCameraModeView& GetCameraModeView() const { return View; }

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

	/** Returns the result of GetPivotRotation, clamped to this camera mode's minimum and maximum pitch and yaw
	 * angles. */
	FRotator GetPivotRotationClamped() const;

	/**
	 * Updates this camera mode's internal view.
	 *
	 * Default implementation simply uses the current pivot location and rotation.
	 */
	virtual void UpdateView(float DeltaTime);

// Properties.
protected:

	/** This camera mode's current view data. */
	FCrashCameraModeView View;

	/** Horizontal field of view. */
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170.0", ClampMin = "5.0", ClampMax = "170.0", Units = "Degrees"))
	float FieldOfView;

	/** Whether to restrict the viewer's vertical camera angle. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Pitch", DisplayName = "Clamp Pitch?")
	bool bClampPitch = true;

	/** Minimum pitch (vertical axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Pitch", DisplayName = "Minimum View Pitch", Meta = (EditCondition = "bClampPitch", UIMin = "-90.0", UIMax = "90", ClampMin = "-90.0", ClampMax = "90", Units = "Degrees"))
	float ViewPitchMin;

	/** Maximum pitch (vertical axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Pitch", DisplayName = "Maximum View Pitch", Meta = (EditCondition = "bClampPitch", UIMin = "-90.0", UIMax = "90.0", ClampMin = "-90.0", ClampMax = "90.0", Units = "Degrees"))
	float ViewPitchMax;

	/** Whether to restrict the viewer's horizontal camera angle. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Yaw", DisplayName = "Clamp Yaw?")
	bool bClampYaw = false;

	/** Minimum yaw (horizontal axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Yaw", DisplayName = "Minimum View Yaw", Meta = (EditCondition = "bClampYaw", UIMin = "0.0", UIMax = "359.9", ClampMin = "0.0", ClampMax = "359.9", Units = "Degrees"))
	float ViewYawMin;

	/** Maximum yaw (horizontal axis) that this camera can view. */
	UPROPERTY(EditDefaultsOnly, Category = "View|Yaw", DisplayName = "Maximum View Yaw", Meta = (EditCondition = "bClampYaw", UIMin = "0.0", UIMax = "359.9", ClampMin = "0.0", ClampMax = "359.9", Units = "Degrees"))
	float ViewYawMax;



	// Blending.

public:

	/** This camera mode's desired blend time. */
	FORCEINLINE float GetBlendTime() const { return BlendTime; }

	/** The current blend weight of this camera mode. 1.0 if this is camera mode is the current focus. 0.0 if this
	 * camera mode is not being viewed. */
	FORCEINLINE float GetBlendWeight() const { return BlendWeight; }

	/** Directly updates the current blend weight. Updates the blend alpha to match. */
	void SetBlendWeight(float Weight);

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

	/** When set, skips all interpolation and puts the camera in the desired position. Gets automatically set to false
	 * next frame. */
	UPROPERTY(Transient)
	uint32 bResetInterpolation : 1;



	// Camera type.

public:

	/** Returns this camera's identifying camera type gameplay tag. E.g. "CameraType.FirstPerson". */
	FGameplayTag GetCameraTypeTag() const { return CameraTypeTag; }

protected:

	/** A tag that can be used to categorize this camera's type. Can be queried for type-specific logic. E.g.
	 * first-person cameras use a "FirstPerson" tag, which is used to determine mesh and effect visibility. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera Type", Meta = (Categories = "CameraType"))
	FGameplayTag CameraTypeTag;



	// Utils.

public:
	
	/** Returns this camera mode's outer object, which should always be a camera component. */
	UCrashCameraComponent* GetCrashCameraComponent() const;

	/** Retrieves the world through the outer object. */
	virtual UWorld* GetWorld() const override;

	/** Helper for retrieving the outer camera component's target actor. */
	AActor* GetTargetActor() const;

	/** Displays information about this camera mode. */
	virtual void DrawDebug(UCanvas* Canvas) const;

#if WITH_EDITOR
	/** Prevents Min and Max clamps from passing each other. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};



/**
 * A stack of camera modes that can be used to blend between them as they are pushed and popped.
 */
UCLASS()
class UCrashCameraModeStack : public UObject
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCameraModeStack();



	// Initialization.

public:

	/** Activates this stack, allowing its cameras to be evaluated. Stacks are active by default, so this should only be
	 * called on stacks that have been deactivated. Stacks begin as active. */
	void ActivateStack();

	/** Deactivates this stack, preventing its cameras from being used. */
	void DeactivateStack();

	/** Returns whether this stack is currently active. */
	FORCEINLINE bool IsStackActivated() const { return bIsActive; }

protected:

	/** Whether this stack is currently active. Cameras are not evaluated while the stack is inactive. */
	bool bIsActive;



	// Camera stack.

public:

	/** Creates a new camera mode instance of the specified type and pushes it to the camera mode stack. */
	void PushCameraMode(TSubclassOf<UCrashCameraModeBase> CameraModeClass);

	/** Continuously called to update this stack's current camera mode and any active blending. */
	bool EvaluateStack(float DeltaTime, FCrashCameraModeView& OutCameraModeView);

	/** Gets the current blend weight and identifying tag of this stack's top (dominant) camera. This is preferable to
	 * checking GetCameraWeightByTag, as it ensures the weighted camera is also the dominant/final camera. */
	void GetBlendInfo(float& OutTopCameraWeight, FGameplayTag& OutTopCameraTag) const;

	/** Returns the current blend weight of the first camera in this stack with the given camera tag. Returns 0.0 if
	 * a camera with the tag is not found. */
	float GetCameraWeightByTag(FGameplayTag CameraTag) const;

	/** Draws debug information about this camera stack's current camera modes. */
	void DrawDebug(UCanvas* Canvas) const;

protected:

	/** Returns the camera mode instance of the specified type in this camera stack. Creates a new one if one does not
	 * exist yet. */
	UCrashCameraModeBase* GetCameraModeInstance(TSubclassOf<UCrashCameraModeBase> CameraModeClass);

	/** Updates each camera mode on the stack. Removes any camera modes that are now irrelevant. */
	void UpdateStack(float DeltaTime);

	/** Blends up the stack, between each weighted camera mode. */
	void BlendStack(FCrashCameraModeView& OutCameraModeView) const;

	/** The internal ordered stack of camera mode instances. */
	UPROPERTY()
	TArray<TObjectPtr<UCrashCameraModeBase>> CameraModeStack;

	/** Unordered collection of the camera mode instances created by this stack. Maintains references to camera mode
	 * instances as they're added to and removed from the stack. */
	UPROPERTY()
	TArray<TObjectPtr<UCrashCameraModeBase>> CameraModeInstances;
};