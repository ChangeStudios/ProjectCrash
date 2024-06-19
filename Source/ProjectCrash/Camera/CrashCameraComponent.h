// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "CrashCameraComponent.generated.h"

class AActor;
class UCrashCameraModeBase;
class UCrashCameraModeStack;

DECLARE_DELEGATE_RetVal(TSubclassOf<UCameraMode>, FDetermineCameraModeSignature);

/**
 * Default camera component for this project. All player-viewable cameras should use this component.
 *
 * This camera uses "camera modes" to dynamically blend between different data-driven views during runtime. It maintains
 * a stack of camera modes: when a new mode is pushed, the camera blends to it; when a mode is popped, the camera blends
 * to the next one on the stack.
 */
UCLASS()
class PROJECTCRASH_API UCrashCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashCameraComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Creates this component's camera stack. */
	virtual void OnRegister() override;



	// Camera modes.

public:

	/** Returns the actor that this camera is currently focused on. Returns the camera's owner by default. */
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	/** Delegate used to query for the desired camera mode. */
	FDetermineCameraModeSignature DetermineCameraModeDelegate;

protected:

	/** Defines this camera's current desired view using the current camera mode. */
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

	UPROPERTY()
	TObjectPtr<UCrashCameraModeStack> CameraModeStack;



	// Utils.

public:

	/** Retrieves the given actor's CrashCameraComponent, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Crash|Camera")
	static UCrashCameraComponent* FindCameraComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UCrashCameraComponent>() : nullptr; }

	/** Debug drawing for this camera's current mode. */
	virtual void DrawDebug(UCanvas* Canvas) const;
};
