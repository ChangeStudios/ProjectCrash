// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "CrashGameplayAbility_AutoRespawn.generated.h"

class UHealthComponent;

/**
 * Listens for when the player's avatar dies via a health component. If possible, the player will try to respawn
 * according to the rules of the game state's PlayerSpawningManagerComponent.
 *
 * This ability can be subclassed to alter its behavior for different game modes and to add user-facing logic (e.g. a
 * respawn timer). Not intended to be subclassed in C++.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashGameplayAbility_AutoRespawn : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

	// Ability logic.

public:

	/** Default constructor. */
	UCrashGameplayAbility_AutoRespawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Starts listening for the ability avatar's death, or starts death logic if the avatar is already dead. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Stops listening for the ability avatar's death. */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Starts listening for the new avatar's death, or starts death logic if the new avatar is already dead. */
	virtual void OnNewAvatarSet() override;



	// Internals.

// Respawn logic.
protected:

	/** Called when this ability's avatar is initialized or changes. Starts listening for the new avatar's death, or
	 * immediately triggers respawn logic if the avatar is already dead. */
	void ActivateOrStartListeningForDeath();

	/** When the avatar dies, starts a respawn timer if the player can respawn. Otherwise, triggers OnRespawnFailed. */
	UFUNCTION()
	void OnDeathStarted(AActor* DyingActor);

	/** Finishes resetting the player when the respawn timer ends, if they haven't been reset already (if a direct
	 * reset was requested during the respawn timer). */
	UFUNCTION()
	void OnRespawnTimerEnd() { if (bShouldFinishReset) { FinishReset(); } }

	/** Restarts the player via the game mode. */
	void FinishReset();

	/** Binds to the current avatar's death via its health component, and to its destruction if it gets destroyed before
	 * its death. */
	void StartListeningForDeath();

	/** Clears any delegates bound to this ability's avatar's death or destruction. Nulls BoundHealthComponent. */
	void StopListeningForDeath();

	/** Called if the player is directly reset during a respawn to cancel the respawn. */
	UFUNCTION()
	void OnResetMessageReceived(FGameplayTag Channel, const FCrashPlayerResetMessage& Message);

	/** Triggers an instant reset from the server if the player's avatar is destroyed without dying. */
	UFUNCTION()
	void OnAvatarEndPlay(AActor* Avatar, EEndPlayReason::Type Reason);

	/** Broadcasts a message indicating that the respawn completed. */
	void OnRespawnCompleted();

// Data.
private:

	/** Whether we've started listening for the player to be reset directly. */
	bool bIsListeningForReset;

	/** Whether we've already finished resetting the player once. Prevents the player from resetting multiple times
	 * (e.g. if a direct reset is requested during a respawn timer). */
	bool bShouldFinishReset;

	/** Listens for a direct reset of the player, which cancels the automatic respawn. */
	FGameplayMessageListenerHandle ResetListener;

	/** The avatar whose destruction we're currently listening for, in case it gets destroyed without dying. */
	UPROPERTY()
	AActor* BoundAvatar;

	/** The health component whose death we're currently listening for. */
	UPROPERTY()
	UHealthComponent* BoundHealthComponent;

	/** The controller we're waiting to reset once the respawn timer finishes. */
	UPROPERTY()
	AController* ControllerToReset;



	// Virtual logic.

protected:

	/**
	 * Returns whether the player can respawn (restart as a player, not a spectator), depending on the game mode rules.
	 *
	 * Default implementation always allows the player to respawn.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool CanRespawn();

	/**
	 * Called when the player tries to respawn, but CanRespawn fails.
	 *
	 * Default implementation changes the player to a spectator, since they can no longer respawn.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnRespawnFailed();

	/** Determines how long the player will wait after dying to respawn. */
	UFUNCTION(BlueprintImplementableEvent)
	float GetRespawnTime();



	// Utils.

public:

	/** Returns true if this ability's avatar is in the "Dying" (DeathStarted) or "Dead" (DeathFinished) state. */
	UFUNCTION(BlueprintPure, Category = "Ability", DisplayName = "Is Avatar Dead or Dying?")
	bool IsAvatarDeadOrDying() const;
};
