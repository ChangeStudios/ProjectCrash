// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameModeBase.generated.h"

/**
 * Base game mode for this project. Handles initialization and transitioning between high-level game phases.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameModeBase : public AModularGameModeBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameModeBase();



	// Initialization.

// Pre-initialization.
public:

	/** Registers this actor as a feature with the initialization state framework. */
	virtual void PreInitializeComponents() override;

	/** Initializes this actor's initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters from initialization states. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

// Initialization states.
public:

	/** The name used to identify this feature (the actor) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

protected:

	/** Handles bound to other actor initialization state changes, e.g. the game state, and the corresponding
	 * actors. */
	TMap<TObjectPtr<AActor>, FDelegateHandle> ActorInitStateChangedHandles;



	// Players.

public:
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
