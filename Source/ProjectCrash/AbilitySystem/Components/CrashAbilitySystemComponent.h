// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashAbilitySystemComponent.generated.h"

/** A generic delegate signature that passes a single gameplay ability pointer. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenericAbilitySignature, UGameplayAbility*, Ability);
/** Delegate used to broadcast when this ASC is granted a new ability. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityGrantedSignature, const FGameplayAbilitySpec&, GrantedAbilitySpec);
/** Delegate used to broadcast when an ability is removed from this ASC. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityRemovedSignature, const FGameplayAbilitySpec&, RemovedAbilitySpec);

/**
 * Tracks the instigator and gameplay effect responsible for the most recent knockback applied to an ASC's avatar.
 * Cleared when the avatar lands or dies.
 */
USTRUCT()
struct FKnockbackSourceData
{
	GENERATED_BODY()

	/** The actor responsible for most recent knockback applied to this ASC's avatar. */
	UPROPERTY()
	TObjectPtr<AActor> KnockbackInstigator;

	/** The gameplay effect definition that applied the most recent knockback to this ASC's avatar. */
	UPROPERTY()
	TObjectPtr<const UGameplayEffect> SourceKnockbackEffect;
};



/**
 * The ability system component used for this project.
 *
 * This project's GAS implementation (like any implementation of GAS) is complex, and may be difficult to understand
 * purely from inline documentation. If you'd like a more digestible explanation of how this framework works, view
 * this document detailing the implementation this system from a higher level:
 *
 *		https://docs.google.com/document/d/1O7YPWexCDY6cmNgkvG8EtkZL657hN82NOsViu8EdND4/edit?usp=sharing
 */
UCLASS()
class PROJECTCRASH_API UCrashAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

	// ----------------------------------------------------------------------------------------------------------------
	//	Construction.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Default constructor. */
	UCrashAbilitySystemComponent();



	// ----------------------------------------------------------------------------------------------------------------
	//	Initialization.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Registers this ASC with the global ability subsystem and attempts to activate passive abilities. If a new, valid
	 * avatar was set for this ASC, informs all abilities of the avatar change. */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/** If the new avatar has a movement component, starts listening for land events, so the current knockback source
	 * can be cleared. */
	virtual void OnNewAvatarSet();

	/** Unregisters this ASC from the global ability subsystem. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	/** Attempts to activate each of this ASC's abilities as a "passive ability." Called when this ASC is initialized
	 * with a new avatar. */
	void TryActivatePassiveAbilities();



	// ----------------------------------------------------------------------------------------------------------------
	//	Input processing.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Called when an input action that corresponds to an ability's input tag is pressed. Queues an appropriate
	* response for when input is processed this frame. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/** Called when an input action that corresponds to an ability's input tag is released. Queues an appropriate
	* response for when input is processed this frame. */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** Processes all ability input for this frame. This is called by the player controller after processing input. */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/** Clears the ability input that occurred this frame. */
	void ClearAbilityInput();

// Internals.
public:

	/** Invokes an "input pressed" event to an ability spec. */
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;

	/** Invokes an "input released" event to an ability spec. */
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

protected:

	/** Abilities whose input was pressed this frame. */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** Abilities whose input was released this frame. */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/** Abilities whose input is currently held. */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;



	// ----------------------------------------------------------------------------------------------------------------
	//	Ability activation groups.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Returns whether the given activation group is currently blocked by ongoing abilities. Independent abilities
	 * are never blocked; exclusive abilities are only blocked if there is an ongoing blocking exclusive ability. */
	bool IsActivationGroupBlocked(EAbilityActivationGroup ActivationGroup) const;

	/** If an exclusive ability is activated, this caches the ability as the current exclusive ability, and cancels the
	 * current exclusive ability, if one exists and is replaceable. Assumes we already checked whether the given
	 * ability's activation group can be activated.
	 */
	void HandleAbilityActivatedForActivationGroup(UCrashGameplayAbilityBase* ActivatedAbility);

	/** If the current exclusive ability ends, clears it as the current exclusive ability, allowing new exclusive
	 * abilities to be activated. */
	void HandleAbilityEndedForActivationGroup(UCrashGameplayAbilityBase* EndedAbility);

protected:

	/** The current exclusive gameplay ability. Null if there is no active exclusive ability. */
	UPROPERTY()
	TObjectPtr<UCrashGameplayAbilityBase> CurrentExclusiveAbility;



	/* Ability events. These are forwarded to the gameplay message subsystem to inform other systems of the events. The
	 * ASC's generic callbacks are insufficient for most cases. */

public:

	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;



	// ----------------------------------------------------------------------------------------------------------------
	//	Gameplay.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Cancels any gameplay abilities that satisfy a given predicate. */
	typedef TFunctionRef<bool(const UCrashGameplayAbilityBase* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	/** Disables all gameplay abilities possessed by this ASC with any of the given tags. Must be called with 
	* authority. */
	void DisableAbilitiesByTag(const FGameplayTagContainer& Tags);

	/** Enables any disabled gameplay abilities possessed by this ASC with any of the given tags. Must be called with 
	* authority. */
	void EnableAbilitiesByTag(const FGameplayTagContainer& Tags);



	// ----------------------------------------------------------------------------------------------------------------
	//	Knockback processing.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Sets the instigator and source for the most recent knockback applied to this ASC's avatar. */
	void SetCurrentKnockbackSource(AActor* Instigator, const UGameplayEffect* KnockbackGE);

	/** Resets current knockback source data. Called when this ASC's avatar lands or dies. */
	void ClearKnockbackSource();

	/** Getter for the instigator of the most recent knockback applied to this ASC's avatar. */
	AActor* GetCurrentKnockbackInstigator() const { return CurrentKnockbackSourceData.KnockbackInstigator; }

	/** Getter for the source effect of the most recent knockback applied to this ASC's avatar. */
	const UGameplayEffect* GetCurrentSourceKnockbackEffect() const { return CurrentKnockbackSourceData.SourceKnockbackEffect; }

protected:

	/** Data tracking the instigator and gameplay effect responsible for the most recent knockback applied to this ASC's
	 * avatar. Cleared when the avatar lands or dies. */
	UPROPERTY()
	FKnockbackSourceData CurrentKnockbackSourceData;



	// ----------------------------------------------------------------------------------------------------------------
	//	Gameplay cues.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Locally executes a gameplay cue on the owning client. */
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/** Add a gameplay cue locally on the owning client. */
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/** Removes a gameplay cue locally from the owning client. */
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);



	// ----------------------------------------------------------------------------------------------------------------
	//	Animation.
	//
	//	Currently, first-person montages are only played locally (but still predicatively). They are NOT replicated to
	//	clients, and animations triggered on local-only abilities are not replicated to the server.
	//
	//	This is because GAS natively supports third-person montages, and adding full support for first-person montages
	//	would require massive modifications to its inner systems. Performing separate bookkeeping for first-person
	//	montages is also difficult, because it would require two separate RPCs for each update, which could bottleneck
	//	our network.
	//
	//  Currently, we have no need to replicate first-person animations.
	//
	//  TODO: Networks tests for viability of separate bookkeeping.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Jumps the current first- and third-person montages to the given section. First-person is not replicated. */
	virtual void CurrentMontageJumpToSection(FName SectionName) override;

	/** Sets the current first- and third-person montages to the next section name. First-person is not replicated. */
	virtual void CurrentMontageSetNextSectionName(FName FromSectionName, FName ToSectionName) override;

	/** Sets the current first- and third-person montages' play rates. First-person is not replicated. */
	virtual void CurrentMontageSetPlayRate(float InPlayRate) override;

	/**
	 * Plays the given montage on this ASC's avatar's first-person mesh, if the avatar is a CrashCharacter. Use
	 * PlayMontage to play a montage on the avatar's third-person mesh.
	 *
	 * First-person montages are only currently only played on local clients, and are NOT replicated.
	 *
	 * @param AnimatingAbility		The ability responsible for playing this animation.
	 * @param ActivationInfo		Activation info used to activate AnimatingAbility.
	 * @param Montage				The montage to play on the first-person mesh.
	 * @param InPlayRate			Montage play-rate.
	 * @param StartSectionName		(Optional) Section of the montage to start playing.
	 * @param StartTimeSeconds		(Optional) Time of the montage from which to start playing.
	 * @return						The remaining duration of the montage. If the montage could not be played (i.e. the
	 *								avatar is null or is not of type CrashCharacter), returns -1.0.
	 */
	float PlayMontage_FirstPerson(UGameplayAbility* AnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f);

protected:

	/** Stops playing a predicted first-person montage is rejected. */
	void OnFirstPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage);

	/** Data about the first-person montage playing locally. */
	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalFirstPersonAnimMontageInfo;



	// ----------------------------------------------------------------------------------------------------------------
	//	Utils.
	// ----------------------------------------------------------------------------------------------------------------

public:

	/** Whether an ability with any of the given tags is currently active on this ASC. */
	bool IsAbilityActive(const FGameplayTagContainer& WithAny);

	/** Whether an ability of the given class is currently active on this ASC. */
	bool IsAbilityActive(TSubclassOf<UCrashGameplayAbilityBase> AbilityClass);

	/** Whether an ability with any of the given tags is currently active on this ASC. */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "Is Ability Active (With Tag)")
	bool IsAbilityActiveByTag(const FGameplayTagContainer& WithAny) { return IsAbilityActive(WithAny); }

	/** Whether an ability of the given class is currently active on this ASC. */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "Is Ability Active (By Class)")
	bool IsAbilityActiveByClass(TSubclassOf<UCrashGameplayAbilityBase> AbilityClass) { return IsAbilityActive(AbilityClass); }

	/** Retrieves the typed version of this ASC's actor info. */
	const FCrashGameplayAbilityActorInfo* GetCrashAbilityActorInfo() const;

	/**
	 * Creates an outgoing gameplay effect spec and adds the given hit result to its effect context.
	 *
	 * You should always use this instead of MakeOutgoingSpec because our damage VFX and knockback systems require
	 * effect contexts to have a hit result.
	 */
	UFUNCTION(BlueprintCallable, Category = GameplayEffects)
	FGameplayEffectSpecHandle MakeOutgoingSpecWithHitResult(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, const FHitResult& HitResult) const;

	/** Shorthand for broadcasting a gameplay ability message. */
	void BroadcastAbilityMessage(const FGameplayTag MessageType, const FGameplayAbilitySpecHandle& Ability, const float Magnitude = 0.0f, bool bReplicateMessage = false);

	/** Multicast for locally broadcasting a gameplay ability message. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastReliableAbilityMessageToClients(const struct FCrashAbilityMessage Message);
};
