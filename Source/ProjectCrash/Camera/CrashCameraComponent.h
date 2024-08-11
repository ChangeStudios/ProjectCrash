// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
#include "CrashCameraComponent.generated.h"

class AActor;
class UCrashCameraModeBase;
class UCrashCameraModeStack;

DECLARE_DELEGATE_RetVal(TSubclassOf<UCrashCameraModeBase>, FDetermineCameraModeSignature);

/**
 * Default camera component for this project. All player-viewable cameras should use this component.
 *
 * This camera uses "camera modes" to dynamically blend between different data-driven views during runtime. It maintains
 * a stack of camera modes: when a new mode is pushed, the camera blends to it; when a mode is popped, the camera blends
 * to the next one on the stack.
 *
 * TODO: Implement first- and third-person camera modes for determining mesh and effects visibility.
 */
UCLASS(Category = "Camera", Meta = (BlueprintSpawnableComponent))
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

	/** The actor that this camera is looking at. Usually the pawn that owns this component. */
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	/** Gets the current blend weight and identifying tag of the current stack's top camera. */
	void GetBlendInfo(float& OutTopCameraWeight, FGameplayTag& OutTopCameraTag) const;

	/** Returns the identifying tag of the current stack's top camera. Useful for checking if we're in first-person. */
	UFUNCTION(BlueprintPure, Category = "Crash|Camera")
	FGameplayTag GetCameraTag() const;

	/** Delegate used to query for the desired camera mode. Bind to this to push camera modes. */
	FDetermineCameraModeSignature DetermineCameraModeDelegate;

protected:

	/** Defines this camera's current desired view using the current camera mode. */
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	/** Checks for any new camera modes that should be pushed to the stack. */
	virtual void UpdateCameraModes();

	/** The camera mode stack handling the camera modes, and blending between them. */
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
