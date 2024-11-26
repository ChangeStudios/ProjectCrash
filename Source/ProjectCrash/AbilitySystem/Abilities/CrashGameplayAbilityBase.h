// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "UIExtensionSystem.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "CrashGameplayAbilityBase.generated.h"

class ACrashCharacter;
class ACrashPlayerState;
class AChallengerBase;
class UAbilityWidgetBase;
class UCrashAbilitySystemComponent;
class UCrashCameraModeBase;

/** Generic gameplay ability delegate with support for dynamic binding.*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicGameplayAbilityDelegate, UGameplayAbility*, Ability);

/**
 * Defines how an ability's activation is triggered, usually with respect to user input.
 */
UENUM(BlueprintType)
enum class EAbilityActivationMethod : uint8
{
	// Activate this ability once when its input is triggered.
	OnInputTriggered,

	/* Repeatedly activate this ability while its input is held. Note that a "Pressed" modifier in an input action will
	 * prevent abilities of this type from behaving as expected. */
	WhileInputActive,

	/*
	 * Immediately activate this ability when (A) this ability is given to an ASC or (B) a new avatar is assigned to the
	 * owning ASC.
	 *
	 * Passive abilities are not automatically deactivated or removed.
	 *
	 * Activation of passive abilities is not predicted.
	 */
	Passive
};



/**
 * Defines how an ability's activation relates to that of other abilities. This is used to ensure certain types of
 * abilities cannot be activated simultaneously.
 */
UENUM(BlueprintType)
enum class EAbilityActivationGroup : uint8
{
	// This ability's activation does not affect and does not rely on other abilities.
	Independent,

	/* This ability cannot be activated while another "exclusive" ability is active. If another exclusive ability is
	 * activated, this ability will be cancelled. */
	Exclusive_Replaceable,

	/* This ability cannot be activated while another "exclusive" ability is active. Other exclusive abilities cannot
	 * be activated until this ability ends. */
	Exclusive_Blocking
};



/**
 * Base gameplay ability class for this project. Extends the base gameplay ability class with additional functionality,
 * integration with this project's input system, and various utilities.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashGameplayAbilityBase(const FObjectInitializer& ObjectInitializer);



	// Input

public:

	/** Getter for this ability's input tag. */
	UFUNCTION(BlueprintPure, Category = "Ability|Activation", Meta = (ToolTip = "The tag corresponding to the input action used to trigger this ability."))
	FGameplayTag GetInputTag() const { return InputTag; }

protected:

	/** The input tag used to trigger this ability's activation via user input. When an input action corresponding to
	 * this tag is triggered, this ability will be activated. Determined by the local player's current input action
	 * mappings. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;



	// Activation.

// Activation method.
public:

	/** Getter for this ability's activation method. */
	UFUNCTION(BlueprintPure, Category = "Ability|Activation", Meta = (ToolTip = "How this ability's activation is triggered."))
	EAbilityActivationMethod GetActivationMethod() const { return ActivationMethod; }

	/** Attempts to activate this ability as a "passive ability." Does not activate if Activation Method is not
	 * Passive. */
	void TryActivatePassiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

protected:

	/** How this ability's activation is triggered. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation", DisplayName = "Ability Activation Method")
	EAbilityActivationMethod ActivationMethod;

// Activation group.
public:

	/** Getter for this ability's activation group. */
	UFUNCTION(BlueprintPure, Category = "Ability|Activation", Meta = (ToolTip = "This ability's activation group."))
	EAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

protected:

	/** How this ability's activation affects or responds to the activation of other abilities. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation", DisplayName = "Ability Activation Group")
	EAbilityActivationGroup ActivationGroup;

// Activation logic.
public:

	/** Checks this ability is disabled or if its activation group is currently blocked. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:

	/** Applies gameplay effects applied by this ability and updates its activation group on the owning ASC. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Removes gameplay effects applied by this ability and updates its activation group on the owning ASC. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;



	// Optional ability widgets.

protected:

	/** When this ability is granted to an ASC, these widgets will be added to their specified slots. The widgets are
	 * removed with the ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TMap<FGameplayTag, TSubclassOf<UUserWidget>> AbilityWidgets;

	/** Handle for this ability's widgets, if it created any. */
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Ability|User Interface")
	TArray<FUIExtensionHandle> AbilityWidgetHandles;



	// Cooldowns.
	
public:

	/** Returns false if a cooldown is blocking the activation of this ability. Ignores cooldowns if the
	 * "bDisableCooldowns" option is enabled in developer settings. */
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:

	/** Tags that can put this ability on a cooldown, in addition to any defined in the cooldown GE class. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns", Meta = (Categories = "Ability.Identifier", EditCondition = "CooldownGameplayEffectClass"))
	FGameplayTagContainer CooldownTags;

	/** The duration of this ability's cooldown. The cooldown GE class must have a set-by-caller duration. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns", Meta = (Units = "seconds", ClampMin = "0"))
	float CooldownDuration;

	/** Tags used as this ability's cooldown tags. This is a union of this ability's CooldownTags and the cooldown GE's
	 * tags. */
	UPROPERTY(Transient)
	FGameplayTagContainer CooldownTags_Internal;

	/** Injects CooldownTags into the cooldown GE, and sets the cooldown GE's duration with CooldownDuration if
	 * possible. */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** Adds this ability's CooldownTags to the tags that can put this ability into a cooldown. */
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** Whether this ability should try to override its cooldown GE's duration. Only true if the cooldown GE is set and
	 * has a set-by-caller duration. */
	bool ShouldSetCooldownDuration() const;



	// Ability event callbacks.

// Internals.
public:

	/** Called when this ability is given to an ASC. */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Called when this ability is removed from an ASC. */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Called when a valid new avatar is set for this ability's owning ASC. */
	virtual void OnNewAvatarSet();

	/** Do not use; Use OnNewAvatarSet instead. OnNewAvatarSet is called for instanced abilities (OnAvatarSet is not)
	 * and is NOT called when the avatar is set to null (OnAvatarSet is). */
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override final {}

// Blueprint-exposed event callbacks.
protected:

	/** Blueprint-implementable version of OnGiveAbility. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On Give Ability", Meta = (ToolTip = "Called when this ability is given to an ASC."))
	void K2_OnGiveAbility();

	/** Blueprint-implementable version of OnRemoveAbility. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On Remove Ability", Meta = (ToolTip = "Called when this ability is removed from an ASC."))
	void K2_OnRemoveAbility();

	/** Blueprint-implementable version of OnAvatarSet. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", DisplayName = "On New Avatar Set", Meta = (ToolTip = "Called when a valid new avatar is set for this ability's owning ASC."))
	void K2_OnNewAvatarSet();



	// Gameplay.

protected:

	/**
	 * Applies knockback to the target actor in the direction of the source to the target.
	 *
	 * @param VelocityMagnitude		Scalar applied to knockback direction, in cm/s.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Knockback")
	void AddKnockbackToTargetFromLocation(float VelocityMagnitude, FVector Source, AActor* Target);

	/**
	 * Applies directional knockback to the target actor.
	 *
	 * @param Velocity				Direction and magnitude of knockback, in cm/s.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Knockback")
	void AddKnockbackToTargetInDirection(FVector Velocity, AActor* Target);

	/** Sets the ability's avatar's camera mode, overriding it temporarily. Requires that the avatar is a pawn with a
	 * PawnCameraManager component. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Camera")
	void SetCameraMode(TSubclassOf<UCrashCameraModeBase> CameraMode);

	/** Clears this ability's overriding camera mode for the ability's avatar. Called automatically when this ability
	 * ends. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Camera")
	void ClearCameraMode();

	/** The camera mode currently being overridden by this ability. */
	TSubclassOf<UCrashCameraModeBase> ActiveCameraMode;



	// Gameplay effects.

protected:

	/** Creates an effect context of type FCrashGameplayEffectContext. */
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;

protected:

	/** Gameplay effects applied to the owning ASC when this ability is activated, and persist after the ability ends.
	 * These effects must be removed manually. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", Meta = (ToolTip = "Effects applied when this ability is activated and persist after it ends. Must be removed manually."))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStart;

	/** Gameplay effects applied to the owning ASC when this ability is activated, and automatically removed when this
	 * ability ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", Meta = (ToolTip = "Effects applied when this ability is activated and automatically removed after it ends."))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStartAndRemoveOnEnd;

	/** Handles to gameplay effects applied from OngoingEffectsToApplyOnStartAndRemoveOnEnd, so they can be removed when
	 * the ability ends. */
	TArray<FActiveGameplayEffectHandle> EffectsToRemoveOnEndHandles;



	// Utils.

public:

	/** Retrieves this ability's current actor info as CrashGameplayAbilityActorInfo. */
	const FCrashGameplayAbilityActorInfo* GetCrashActorInfo() const;

	/** Blueprint-exposed accessor for typed actor info. Returns a copy instead of a pointer. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info", DisplayName = "GetCrashActorInfo", Meta = (ToolTip = "Retrieves this ability's typed actor info."))
	FCrashGameplayAbilityActorInfo K2_GetCrashActorInfo() const;

	/** Returns this ability's owning ASC as a CrashAbilitySystemComponent. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponentFromActorInfo() const;

	/** Returns this ability's avatar as a CrashCharacter. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info")
	ACrashCharacter* GetCrashCharacterFromActorInfo() const;

	/** Returns the typed player controller from the current actor info. Often null. To try to retrieve any controller,
	 * use GetControllerFromActorInfo instead. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info")
	ACrashPlayerController* GetCrashPlayerControllerFromActorInfo() const;

	/** Attempts to retrieve any controller responsible for this ability: the owning actor's PC, the avatar's
	 * controller, etc. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info")
	AController* GetControllerFromActorInfo() const;

	/** Attempts to retrieve the typed player state from the current actor info. Often null. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Actor Info")
	ACrashPlayerState* GetCrashPlayerStateFromActorInfo() const;

	/** This ability has these tags. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	const FGameplayTagContainer& GetAbilityTags() const { return AbilityTags; }

	/** A gameplay target data filter that filters for actors with an ability system component. Uses this ability's
	 * avatar for "self" filtering. */
	UFUNCTION(BlueprintPure, Category = "Ability Statics", DisplayName = "GAS Actors Filter")
	FGameplayTargetDataFilterHandle MakeGASActorFilter(ETargetDataFilterSelf::Type FilterSelf);

	/** Returns this ability's CDO. */
	UE_DEPRECATED(0.2.3, TEXT("Why are are we using the CDO?"))
	FORCEINLINE const UCrashGameplayAbilityBase* GetAbilityCDO() const { return GetClass()->GetDefaultObject<UCrashGameplayAbilityBase>(); }



	// Validation.

public:

#if WITH_EDITOR

	/** Disables support for bReplicateInputDirectly and ensures CooldownDuration is greater than 0, if set. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;

#endif // WITH_EDITOR



	// Editor.

public:

#if WITH_EDITOR

	/** Determines whether certain properties in this class can be edited. */
	virtual bool CanEditChange(const FProperty* InProperty) const override;

#endif // WITH_EDITOR
};
