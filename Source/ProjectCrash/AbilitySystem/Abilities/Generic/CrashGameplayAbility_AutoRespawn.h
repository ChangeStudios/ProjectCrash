// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashGameplayAbility_AutoRespawn.generated.h"

class UHealthComponent;

/**
 * Listens for when the player's avatar dies via a health component. If possible, the player will try to respawn.
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

	void ActivateOrStartListeningForDeath();

	UFUNCTION()
	void OnDeathStarted(AActor* DyingActor);

	UFUNCTION()
	void OnRespawnTimerEnd() { if (bShouldFinishReset) { FinishReset(); } }

	void FinishReset();

	/** Binds to the current avatar's death via its health component, and to its destruction if it gets destroyed before
	 * its death. */
	void StartListeningForDeath();

	/** Clears any delegates bound to this ability's avatar's death or destruction. Nulls BoundHealthComponent. */
	void StopListeningForDeath();

	/** Triggers an instant reset from the server if the player's avatar is destroyed without dying. */
	UFUNCTION()
	void OnAvatarEndPlay(AActor* Avatar, EEndPlayReason::Type Reason);

// Data.
private:

	bool bIsListeningForReset;

	bool bShouldFinishReset;

	UPROPERTY()
	AActor* BoundAvatar;

	UPROPERTY()
	UHealthComponent* BoundHealthComponent;

	UPROPERTY()
	AController* ControllerToReset;



	// Virtual logic.

protected:

	/**
	 * Returns whether this ability's player can respawn.
	 *
	 * Default implementation return true if the player's "Lives" attribute (ULivesAttributeSet) is at least one.
	 * Returns false if the player has no lives left, or does not have a LivesAttributeSet.
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool CanRespawn();

	/**
	 * Called when the player attempts to respawn but CanRespawn fails.
	 *
	 * Default implementation makes the respawning player a spectator.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnRespawnFailed();

	/**
	 * Returns how long the player should wait after dying to respawn.
	 *
	 * Default implementation returns the "RespawnTime" game mode property.
	 */
	UFUNCTION(BlueprintNativeEvent)
	float GetRespawnTime();



	// Utils.

public:

	/** Returns true if this ability's avatar is in the "Dying" (DeathStarted) or "Dead" (DeathFinished) state. */
	UFUNCTION(BlueprintPure, Category = "Ability", DisplayName = "Is Avatar Dead or Dying?")
	bool IsAvatarDeadOrDying() const;
};
