// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashPlayerStateBase.generated.h"

class UChallengerData;
class UChallengerSkinData;

/**
 * Base player state for this project. Handles generic initialization.
 *
 * This is only used as a base for gameplay player states. Other player states (e.g. main menu, character selection,
 * etc.) don't require initialization logic this complex, nor do they need Challenger or skin data.
 */
UCLASS()
class PROJECTCRASH_API ACrashPlayerStateBase : public AModularPlayerState, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
	
	// Initialization.

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



	// Challenger data.

public:

	/** Accessor for CurrentChallenger. */
	FORCEINLINE const UChallengerData* GetCurrentChallenger() const;

protected:

	/** The challenger currently selected by this player. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Challenger Data")
	TSoftObjectPtr<UChallengerData> CurrentChallenger;



	// Skin data.

public:

	/** Accessor for CurrentSkin. */
	FORCEINLINE const UChallengerSkinData* GetCurrentSkin() const;

protected:

	/** The current skin being used for the current Challenger. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Challenger Data")
	TSoftObjectPtr<UChallengerSkinData> CurrentSkin;
};
