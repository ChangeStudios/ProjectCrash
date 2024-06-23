// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponGameplayAbility.h"
#include "MeleeAttackAbility.generated.h"

class AGameplayAbilityTargetActor_Trace;
class AGameplayAbilityTargetActor_SingleLineTrace;
class AGameplayAbilityTargetActor_CollisionDetector_Capsule;


/**
 * Defines the targeting method used for a melee ability. It'll take some play-testing to figure out which of these
 * works best for this project. Once we do, we'll likely only be using one for most of the melee abilities.
 */
UENUM(BlueprintType)
enum class EMeleeTargetingType : uint8
{
	/**
	 * A capsule collision test is performed at exactly one point during the attack: when the Event.Ability.MeleeAttack
	 * event is received.
	 *
	 * When using instant targeting, ensure you're only using one set of events (usually just one) per animation pair.
	 * I.e. do not fire events in both the first-person and third-person montages; only fire them in one. Since both
	 * are played on the server, it does not matter which one you choose.
	 */
	Instant,

	/**
	 * Begins targeting when the "MeleeAttack.Start" event is received, and stops targeting when "MeleeAttack.Stop" is
	 * received.
	 * 
	 * Warning: Do not use; not currently supported. TODO: Implement.
	 */
	EventDuration,

	/** A target actor is used to perform continuous collision testing while the ability is active (i.e. while the
	 * montage is playing). */
	EntireDuration,
};



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
UCLASS(Abstract)
class PROJECTCRASH_API UMeleeAttackAbility : public UWeaponGameplayAbility
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UMeleeAttackAbility(const FObjectInitializer& ObjectInitializer);



	// Ability logic.

public:

	/** Binds debug info to ability system debugger. */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Plays the subsequent attack animation and waits for target data. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Ends targeting if duration-based targeting is being used. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;



	// Targeting.

protected:

	/** Called when an actor with an ASC is hit by this melee attack ability. Line-of-sight has already been checked at
	 * this point. EffectContext contains the hit location in HitResult. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetHit(UAbilitySystemComponent* TargetASC, FGameplayEffectContextHandle EffectContext);

private:

	/** Performs ability logic with target data once it's received. Checks line-of-sight, triggers an optional impact
	 * cue at the impact point, and executes blueprint logic for applying effects. Used for both instant and
	 * duration-based targeting. */
	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData);



	// Hit detection.

protected:

	/** The distance from the user's camera to which collision will be checked. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection", Meta = (ForceUnits = "cm"))
	float AttackRange;

	/** The radius of the collision capsule used to detect hits. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection", Meta = (ForceUnits = "cm"))
	float AttackRadius;

	/** Type of melee targeting to perform. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Hit Detection")
	EMeleeTargetingType TargetingType;

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
	 * hit. This will be overridden by any matching cue in the ability owner's current skin's AbilityCues array. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects", Meta = (Categories = "GameplayCue"))
	FGameplayTag DefaultHitImpactCue;

	/** Optional gameplay cue fired at the end of an attack if the attack did not hit any ability system actors, but
	 * hit a surface. This will be overridden by any matching cue in the ability owner's current skin's AbilityCues
	 * array. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack|Effects", Meta = (Categories = "GameplayCue"))
	FGameplayTag DefaultSurfaceImpactCue;



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

	/** Begins targeting with TargetActor. */
	UFUNCTION()
	void StartTargeting();

	/** Ends targeting with TargetActor. */
	UFUNCTION()
	void EndTargeting();

	/** Performs a forward trace to attempt to hit a surface, triggered when an attack misses. */
	UFUNCTION()
	void TryHitSurface(FGameplayEventData Payload);

// Instant targeting.
private:

	/** Performs predictive instant targeting for this ability. */
	UFUNCTION()
	void PerformInstantTargeting(FGameplayEventData Payload);

	/** Performs a sphere trace from the owning player with AttackRadius radius. Only used for instant targeting. */
	bool PerformTrace(TArray<FHitResult>& OutHitResults) const;

	/** Callback triggered when this ability has performed its local targeting. Only used for instant targeting;
	 * duration-based targeting uses a target actor. */
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	/** Handle for callback to OnTargetDataReady. */
	FDelegateHandle OnTargetDataReadyDelegateHandle;

// Helpers.
private:

	/** Calculates the position of the trace capsule, with or without the end-point radius. This can be used to get
	 * the target actor's position, or to generate data for a sphere trace that simulates the target actor's capsule. */
	void GetCapsulePosition(bool bIncludeRadius, FVector& Base, FVector& Top) const;

// Debugging.
private:

	bool bGASDebugEnabled;
};
