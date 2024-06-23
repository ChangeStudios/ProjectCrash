// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "PawnCameraManager.generated.h"

class UCrashCameraModeBase;

/**
 * Sets up camera handling for pawns. The owning pawn should possess a CrashCameraComponent.
 *
 * This component requires that its owning pawn has a pawn extension component for proper initialization.
 */
UCLASS(NotBlueprintable, Category = "Camera", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UPawnCameraManager : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UPawnCameraManager(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Registers this component with the initialization state framework. */
	virtual void OnRegister() override;

	/** Initializes this components initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters this component from the initialization state framework. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Initialization states.

public:

	/** The name used to identify this feature (the component) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;



	// Camera handling.

public:

	/** Bound to the camera component to determine which camera mode to use. Uses the ability camera mode, if there is
	 * one. Otherwise, uses the pawn's default camera mode. */
	TSubclassOf<UCrashCameraModeBase> DetermineCameraMode() const;



	// Ability-driven camera mode.

public:

	/** Overrides the current camera mode from an active gameplay ability. */
	void SetAbilityCameraMode(TSubclassOf<UCrashCameraModeBase> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the current ability camera mode override, if there is one. */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:

	/** The overriding camera mode set by an ability. */
	UPROPERTY()
	TSubclassOf<UCrashCameraModeBase> AbilityCameraMode;

	/** Spec handle for the last ability to set the camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;



	// Utils.

public:

	/** Retrieves the given actor's PawnCameraManager, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Crash|Camera")
	static UPawnCameraManager* FindPawnCameraManagerComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UPawnCameraManager>() : nullptr; }
};
