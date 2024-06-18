// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "Input/CrashInputActionMapping.h"
#include "InputHandlerComponentBase.generated.h"

class UCrashInputActionMapping;

/**
 * Defines input for pawns. Applies input configurations and defines handlers for gameplay abilities. This component
 * should be subclassed to define pawn-specific input handlers.
 *
 * This component requires that its owning pawn has a pawn extension component for proper initialization.
 */
UCLASS(Blueprintable)
class PROJECTCRASH_API UInputHandlerComponentBase : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Initialization.

protected:

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



	// Input.

public:

	void AddAdditionalInputActions(const UCrashInputActionMapping* InputActionMapping);

	void RemoveAdditionalInputActions(const UCrashInputActionMapping* InputActionMapping);

	bool IsReadyToBindInput() const { return bReadyToBindInputs; }

	static const FName NAME_BindInputsNow;

protected:

	bool bReadyToBindInputs;
};
