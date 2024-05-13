// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "MeleeAttackAbility.generated.h"

class AGameplayAbilityTargetActor_CollisionDetector_Capsule;
/**
 * A standard melee attack. When activated, a collision detector capsule will be drawn ahead of the user's camera to
 * check for any targets for the duration of the ability. This is intended for primary attacks that can be performed
 * repeatedly in succession without a cooldown; it has not been tested for use with one-off abilities.
 *
 * A series of first- and third-person attack animations must be defined to play when the ability is activated. Each
 * animation will be played in a looping order with each subsequent attack, until the attack sequence is reset (i.e.
 * the user stops attacking).
 *
 * By default, hit detection is performed the entire duration the ability is active. This behavior can be overridden
 * with the bDetectEntireDuration property, and by sending Event.Ability.MeleeAttack.Start and .End events to this
 * ability during each animation.
 */
UCLASS()
class PROJECTCRASH_API UMeleeAttackAbility : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UMeleeAttackAbility(const FObjectInitializer& ObjectInitializer);

	/** Plays the subsequent attack animation and waits for target data. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	/** Called when an actor with an ASC is hit by this melee attack ability. Line-of-sight has already been checked at
	 * this point. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetHit(UAbilitySystemComponent* TargetASC);

private:

	/** Checks line-of-sight and triggers an optional impact cue at the impact point. */
	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data);



	// Hit detection.

protected:

	/** The distance from the user's camera to which collision will be checked. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection", Meta = (ForceUnits = "cm"))
	float AttackRange;

	/** The radius of the collision capsule used to detect hits. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection", Meta = (ForceUnits = "cm"))
	float AttackRadius;

	/** Whether to perform hit detection for the entirety of each animation. If false, the
	 * Event.Ability.MeleeAttack.Start event tag must be sent to the owning ASC to instruct this ability to begin
	 * targeting. MeleeAttack.End can be used to end targeting; otherwise targeting will end when this ability ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection")
	bool bDetectEntireDuration;



	// Animation.

protected:

	/** The ordered sequence of first-person attack montages used for this melee ability. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Attack Montages", Category = "Melee Attack|Animations")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages_FPP;

	/** The ordered sequence of third-person attack montages used for this melee ability. These will be played in the
	 * given order with each subsequent attack. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Attack Montages", Category = "Melee Attack|Animations")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages_TPP;



	// Effects.

protected:

	/** Optional gameplay cue fired when an ability system actor is hit. This cue is triggered at the location of the
	 * hit. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects")
	FGameplayCueTag HitImpactCue;

	/** Optional gameplay cue fired at the end of an attack if the attack did not hit any ability system actors, but
	 * hit a surface. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects")
	FGameplayCueTag SurfaceImpactCue;



	// Utils.

// Internals.
private:

	/** The target actor used to detect hits. */
	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor_CollisionDetector_Capsule> TargetActor;

	/** Tracks the last time this ability was used. When this ability is activated, the attack animation sequence will
	 * be reset if a sufficiently long amount of time has passed since the ability was last activated. */
	float LastUsed;

	/** The amount of time succeeding an attack, after which we will reset our sequence of attack animations. */
	float TimeBeforeReset;

	/** The index of the next first-person attack animation we'll play in the sequence. */
	int32 CurrentAttackAnim_FPP;

	/** The index of the next third-person attack animation we'll play in the sequence. */
	int32 CurrentAttackAnim_TPP;

	/** Tracks whether this instance of the ability hit any targets. Used to determine if we should perform a hit
	 * test on a surface instead. */
	bool bHitTargets;

// Targeting logic.
private:

	/** Starts or ends targeting, when the Start or End events are received. */
	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);

	/** Begins targeting with TargetActor. */
	UFUNCTION()
	void StartTargeting();

	/** Ends targeting with TargetActor. */
	UFUNCTION()
	void EndTargeting();

// Helpers.
private:

	/** Wraps EndAbility so it can be called via delegate from PlayDualMontageAndWait. */
	UFUNCTION()
	void EndAbilityWrapper();
};
