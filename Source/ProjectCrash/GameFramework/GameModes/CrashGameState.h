// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "ModularGameState.h"
#include "CrashGameState.generated.h"

struct FCrashAbilityMessage;
struct FCrashVerbMessage;
class ACrashPlayerState;
class UCrashAbilitySystemComponent;
class UCrashGameModeData;
class UGameModeManagerComponent;
namespace UE::GameFeatures { struct FResult; }

/**
 * Base modular game state for this project. This is the primary handler for game initialization, responsible for
 * loading the modular game mode data and executing its startup routines (executing actions, loading assets, etc.) via
 * the GameModeManagerComponent.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameState : public AModularGameStateBase, public IGameFrameworkInitStateInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Registers this actor as a feature with the initialization state framework. */
	virtual void PreInitializeComponents() override;

	/** Starts listening for the game mode to be loaded via the game mode manager component. Initializes the ASC. */
	virtual void PostInitializeComponents() override;

	/** Initializes this actor's initialization state. */
	virtual void BeginPlay() override;

	/** Unregisters this actor from the initialization state framework. */
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



	// Game mode.

private:

	/** Handles loading and managing the current game mode from a game mode data asset. */
	UPROPERTY()
	TObjectPtr<UGameModeManagerComponent> GameModeManagerComponent;



	// Ability system.

public:

	/** Interfaced getter for the game state's ability system component. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Typed getter for the game state's ability system component. */
	UFUNCTION(BlueprintCallable, Category = "Game State", Meta = (ToolTip = "The game state's ability system"))
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:

	/** The game state's ability system component, used for managing game-wide abilities (e.g. game phases), effects,
	 * and cues. */
	UPROPERTY(VisibleAnywhere, Category = "Game State")
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;



	// Players.

public:

	/** Checks if all expected players have joined the game before the game can begin. */
	virtual void AddPlayerState(APlayerState* PlayerState) override;



	// Messaging.

public:

	/** Unreliably broadcasts a verbal message to all clients. */
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Messaging", DisplayName = "Broadcast Message to Clients (Unreliable)")
	void MulticastUnreliableMessageToClients(const FCrashVerbMessage Message);

	/** Reliably broadcasts a verbal message to all clients. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Messaging", DisplayName = "Broadcast Message to Clients (Verb, Reliable)")
	void MulticastReliableMessageToClients(const FCrashVerbMessage Message);



	// Utils.

private:

	/**
	 * Determines how many players should be in this game. The game will wait until this many players have logged in
	 * and been initialized before starting. Also checks for any disconnects.
	 *
	 * Uses session properties for standalone runs. Uses the editor's "Number of Players" setting during PIE.
	 */
	int32 GetNumExpectedPlayers() const;

	/** Gets the number of active players (all players excluding inactive players and spectators). */
	int32 GetNumActivePlayers() const;
};
