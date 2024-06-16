// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PlayerStateComponent.h"
#include "IntroCinematicComponent.generated.h"

class ALevelSequenceActor;

/**
 * Plays the current level's intro cinematic on a loop until the player state is ready to transition to the
 * Initializing state. Prevents the initialization state from progressing until the first loop finishes. Will skip the
 * cinematic if the game has already started (i.e. joined late).
 */
UCLASS(NotBlueprintable)
class PROJECTCRASH_API UIntroCinematicComponent : public UPlayerStateComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Cinematic.

public:

	/** Progresses initialization state. Resets the cinematic and starts playing it in a loop. */
	UFUNCTION()
	void OnFirstLoopFinished();

private:

	/** Handle to the level sequence actor playing the current map's intro cinematic. Null if the intro cinematic is
	 * not playing. */
	UPROPERTY()
	ALevelSequenceActor* IntroCinematicSequenceActor;

	/** Whether the cinematic's first loop has finished playing. Initialization of this component cannot progress until
	 * this is true. */
	bool bFinishedFirstLoop = false;



	// Initialization.

public:

	/** Registers this actor as a feature with the initialization state framework. */
	virtual void OnRegister() override;

	/** Initializes this actor's initialization state. Starts playing the current map's intro cinematic. */
	virtual void BeginPlay() override;

	/** Unregisters this actor as an initialization state feature. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Initialization states.

public:

	/** The name used to identify this feature (the actor component) during initialization. */
	static const FName NAME_ActorFeatureName;
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }

	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
};
