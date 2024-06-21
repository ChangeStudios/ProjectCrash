// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "CrashAbilitySystemComponent.generated.h"

/**
 * Defines from where a gameplay ability was granted.
 */
UENUM()
enum EAbilitySource : uint8
{
	Other,
	Challenger,
	Equipment
};

/** A generic delegate signature that passes a single gameplay ability pointer. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenericAbilitySignature, UGameplayAbility*, Ability);
/** Delegate used to broadcast when this ability system is initialized. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FASCInitSignature, AActor*, OwnerActor, AActor*, AvatarActor);
/** Delegate used to broadcast when this ASC is granted a new ability. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityGrantedSignature, const FGameplayAbilitySpec&, GrantedAbilitySpec);
/** Delegate used to broadcast when an ability is removed from this ASC. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityRemovedSignature, const FGameplayAbilitySpec&, RemovedAbilitySpec);
/** Delegate used to broadcast the Death event and communicate in important information. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathEventSignature, const FDeathData&, DeathData);

/**
 * The ability system component used for this project.
 *
 * This project's GAS implementation (like any implementation of GAS) is complex and may be difficult to understand
 * purely from inline documentation. If you'd like a more digestible explanation of how this framework functions, view
 * this document detailing the implementation this system from a higher level:
 *
 *		https://docs.google.com/document/d/1lrocajswgfGHrTl-TFuM-Iw5N444_QWDiKfILris98c/edit?usp=sharing
 */
UCLASS()
class PROJECTCRASH_API UCrashAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashAbilitySystemComponent();



	// Initialization.

public:

	/** Registers this ASC with the global ability system and broadcasts OnInit. */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/** Delegate broadcast when this ASC is initialized with a new actor (e.g. after respawning). */
	UPROPERTY()
	FASCInitSignature InitDelegate;



	// Uninitialization.

public:

	/** Unregisters this ASC from the global ability system. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Utilities.

public:

	/** Returns the actor currently acting as this ASC's avatar. Blueprint-exposed wrapper for GetAvatarActor(). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability System|Utilities", Meta = (DisplayName = "Get Avatar Actor"))
	AActor* K2_GetAvatarActor() { return GetAvatarActor(); };



	// Ability management.

public:

	/** Delegate fired when a new ability is granted to this ASC. */
	UPROPERTY()
	FAbilityGrantedSignature AbilityGrantedDelegate;

	/** Delegate fired when an ability is removed from this ASC. */
	UPROPERTY()
	FAbilityRemovedSignature AbilityRemovedDelegate;

protected:

	/** Fires the AbilityGrantedDelegate when an ability is granted to this ASC. */
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	/** Fires the AbilityRemovedDelegate when an ability is removed from this ASC. */
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;



	// Ability activation.

public:

	/** Returns whether the given activation group is currently blocked by ongoing abilities. Independent abilities
	 * are never blocked; exclusive abilities are only blocked if there is an ongoing blocking exclusive ability. */
	bool IsActivationGroupBlocked(EAbilityActivationGroup ActivationGroup) const;

	/** Caches the given ability if it's an exclusive ability and can override the current exclusive ability. If it can,
	 * the current exclusive ability will be cancelled. */
	void HandleAbilityActivatedForActivationGroup(UCrashGameplayAbilityBase* ActivatedAbility);

	/** Clears the given ability from the current exclusive ability cache if it's exclusive, allowing new exclusive
	 * abilities to be activated. */
	void HandleAbilityEndedForActivationGroup(UCrashGameplayAbilityBase* EndedAbility);

protected:

	/** The current ongoing exclusive gameplay ability. */
	UPROPERTY()
	TObjectPtr<UCrashGameplayAbilityBase> CurrentExclusiveAbility;



	// Ability notifications.

public:

	/** Broadcasts a message communicating that the ability successfully activated. */
	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;

	/** Broadcasts a message communicating that the ability ended. */
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	/** Broadcasts a message communicating the ability activation failure. */
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;



	// Gameplay cues.

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



	// Animation.

public:

	/**
	 * Plays the given montage on this ASC's avatar's first-person mesh, if the avatar is a ACrashCharacterBase.
	 * Otherwise, returns -1.0.
	 *
	 * This does not affect the ability system's animation data, such as LocalAnimMontageInfo. The ability system's
	 * animation data is only affected by third-person animations, since first-person animations are rarely relevant to
	 * anyone besides the local client.
	 */
	float PlayFirstPersonMontage(UGameplayAbility* AnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f);

protected:

	/** Called when a prediction key that played a first-person montage is rejected. */
	void OnFirstPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage);



	// Death.

public:

	/** Delegate broadcast when a Death ability is activated to communicate death data. */
	UPROPERTY()
	FDeathEventSignature DeathEventDelegate;

};
