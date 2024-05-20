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
 * By default, hit detection is performed once during an attack, when the Event.Ability.PerformTargeting event is
 * received by the owning ASC. The bUseInstantTargeting property can be disabled to switch to duration-based targeting,
 * which will use a target actor to continuously perform hit detection while the ability is active.
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

	/** Ends targeting if duration-based targeting is being used. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	/** Called when an actor with an ASC is hit by this melee attack ability. Line-of-sight has already been checked at
	 * this point. EffectContext contains the hit location in HitResult. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetHit(UAbilitySystemComponent* TargetASC, FGameplayEffectContextHandle EffectContext);

private:

	/** Checks line-of-sight and triggers an optional impact cue at the impact point. Used for both instant and
	 * duration-based targeting. */
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

	/**
	 * Whether to perform instantaneous targeting or duration-based targeting.
	 *
	 * * Instantaneous Targeting: A capsule collision test is performed at exactly one point during the attack: when the
	 * Event.Ability.MeleeAttack event is received.
	 *
	 * When using instant targeting, ensure you're only using one set of events (usually just one) per animation pair.
	 * I.e. do not fire events in both the first-person and third-person montages; only fire them in one. Since both
	 * are played on the server, it does not matter which one you choose.
	 *
	 * * Duration-Based Targeting: A target actor is used to perform continuous collision testing while the ability is
	 * active.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection")
	bool bUseInstantTargeting;

	/** TODO: Implement event-based durations that can be controlled by gameplay events—i.e. "MeleeAttack.Start" and
	 * "MeleeAttack.Stop." */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection", Meta = (EditCondition = "bUseInstantTargeting == false", EditConditionHides = "true", ToolTip = "WARNING: Not implemented; Do not use!"))
	bool bUseEventBasedDuration;

	/** Any targets with any of these tags will be ignored. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection")
	FGameplayTagContainer IgnoreTargetsWithTags;



	// Animation.

protected:

	/** The ordered sequence of first-person attack montages used for this melee ability. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "First-Person Attack Montages", Category = "Melee Attack|Animations")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages_FPP;

	/** The ordered sequence of third-person attack montages used for this melee ability. */
	UPROPERTY(EditDefaultsOnly, DisplayName = "Third-Person Attack Montages", Category = "Melee Attack|Animations")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages_TPP;



	// Effects.

protected:

	/** Optional gameplay cue fired when an ability system actor is hit. This cue is triggered at the location of the
	 * hit. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects", Meta = (Categories = "GameplayCue"))
	FGameplayTag HitImpactCue;

	/** Optional gameplay cue fired at the end of an attack if the attack did not hit any ability system actors, but
	 * hit a surface. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects", Meta = (Categories = "GameplayCue"))
	FGameplayTag SurfaceImpactCue;



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
	void OnPerformTargetingReceived(FGameplayEventData Payload);

	/** Begins targeting with TargetActor. */
	UFUNCTION()
	void StartTargeting();

	/** Ends targeting with TargetActor. */
	UFUNCTION()
	void EndTargeting();

	/** Performs a forward trace to attempt to hit a surface. */
	UFUNCTION()
	void TryHitSurface(FGameplayEventData Payload);

// Helpers.
private:

	/** Calculates the position of the trace capsule, with or without the end-point radius. */
	void GetCapsulePosition(bool bIncludeRadius, FVector& Base, FVector& Top);

	/** Wraps EndAbility so it can be called via delegate from PlayDualMontageAndWait. */
	UFUNCTION()
	void EndAbilityWrapper();
};
