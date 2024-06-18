// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "Input/CrashInputActionMapping.h"
#include "InputHandlerComponentBase.generated.h"

class UCrashInputComponent;
class UCrashInputActionMapping;
struct FInputActionValue;

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

	/** Binds all ability actions in the given mapping. Note that this does NOT bind any native actions defined in the
	 * given action mapping. */
	void AddAdditionalInputActions(const UCrashInputActionMapping* ActionMapping);

	/** Unbinds all ability actions in the given mapping, if the mapping is currently bound. */
	void RemoveAdditionalInputActions(const UCrashInputActionMapping* ActionMapping);

	/** Whether this component is ready to bind input. Used to determine when to add modular input bindings. */
	bool IsReadyToBindInput() const { return bReadyToBindInputs; }

protected:

	/** Initializes the given input component with this component's handlers and its owning pawn's pawn data. */
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	/** Binds the handler functions defined in this component with the appropriate input action. Override this to bind
	 * handler functions defined by pawn-specific subclasses of this component. */
	virtual void BindInputHandlers(UCrashInputComponent* CrashIC, const UCrashInputActionMapping* ActionMapping) {};

	/** Event name for when the modular game framework should bind modular, game mode-specific input. */
	static const FName NAME_BindInputsNow;

protected:

	/** Whether input can currently be bound with this component. */
	bool bReadyToBindInputs;
};
