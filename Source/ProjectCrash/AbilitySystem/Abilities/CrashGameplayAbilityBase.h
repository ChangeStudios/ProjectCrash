// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CrashGameplayAbilityBase.generated.h"

class UAbilityTask_WaitInputRelease;
class AChallengerBase;
class UCrashAbilitySystemComponent;

/** Generic gameplay ability delegate with support for dynamic binding.*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicGameplayAbilityDelegate, UGameplayAbility*, Ability);

/**
 * Defines how an ability's activation relates to that of other abilities. This is used to ensure certain types of
 * abilities cannot be activated simultaneously.
 */
UENUM(BlueprintType)
enum class EAbilityActivationGroup : uint8
{
	// This ability's activation does not affect and does not rely on other abilities.
	Independent,

	/* This ability cannot be activated while another "exclusive" ability is active. This ability can be cancelled and
	 * replaced by other exclusive abilities. */
	Exclusive_Replaceable,

	/* This ability cannot be activated while another "exclusive" ability is active. This ability can never be
	 * cancelled and replaced by other exclusive abilities. */
	Exclusive_Blocking
};



/**
 * The ability type displayed to players, e.g. in "ability info" screens.
 */
UENUM(BlueprintType)
enum class EFrontendAbilityType : uint8
{
	PrimaryWeapon,
	SecondaryWeapon,
	StandardAbility,
	PassiveAbility,
	UltimateAbility
};



/**
 * The base class for gameplay abilities in this project. Extends the base gameplay ability class with additional
 * functionality and various helper functions.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashGameplayAbilityBase(const FObjectInitializer& ObjectInitializer);



	// Utilities.

public:

	/** Returns this ability's instigating ASC if it is a CrashAbilitySystemComponent. Returns nullptr if the ASC was
	 * not found or is of the wrong class. */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Utilities")
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponentFromActorInfo() const;

	/** Returns this ability's avatar actor if it is a subclass of the base challenger character class. Returns nullptr
	 * if the avatar could not be found or is of the wrong class. */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Utilities")
	AChallengerBase* GetChallengerFromActorInfo() const;

	/** Returns this ability's CDO. */
	FORCEINLINE const UCrashGameplayAbilityBase* GetAbilityCDO() const { return GetClass()->GetDefaultObject<UCrashGameplayAbilityBase>(); }

#if WITH_EDITOR

	/** Define when certain properties can be edited in this ability's archetype. */
	virtual bool CanEditChange(const FProperty* InProperty) const override;

#endif // #if WITH_EDITOR



	// Activation behavior.

public:

	/** Getter for this ability's input tag. */
	UFUNCTION(BlueprintPure, Category = "Ability System|Ability Activation", Meta = (ToolTip = "This tag will be bound to corresponding input actions to trigger this ability."))
	const FGameplayTag& GetInputTag() const { return InputTag; }

	/** Getter for this ability's activation group. */
	UFUNCTION(BlueprintPure, Category = "Ability System|Ability Activation", Meta = (ToolTip = "How this ability's activation affects and relies on other abilities."))
	EAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

protected:

	/** This tag will be bound to corresponding input actions to trigger this ability. This is bound by the owning
	 * ability set when it's granted. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	/** How this ability's activation affects and relies on other abilities. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation", DisplayName = "Ability Activation Group")
	EAbilityActivationGroup ActivationGroup;



	// Ability activation requisites.

public:

	/** Checks this ability is disabled or if its activation group is currently blocked. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;



	// Ability activation.

protected:

	/** Applies gameplay effects applied by this ability and updates its activation group on the owning ASC. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	/** Removes gameplay effects applied by this ability and updates its activation group on the owning ASC. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;



	// Ability cooldowns.

protected:

	/** Sends a gameplay message when this ability's cooldown is applied. */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;



	// User interface.

public:

	/** This ability's icon in the ability bar. Abilities must have an AbilityBehavior tag enabling their appearance in
	 * the user interface before the ability will appear. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	TObjectPtr<UTexture2D> AbilityIcon;

	/** Display name of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	FString DisplayedAbilityName;

	/** User-facing type of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	EFrontendAbilityType FrontendAbilityType;

	/** User-facing description of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "User Interface")
	FString DisplayedAbilityDescription;



	// Blueprint-implementable callback functions.

// Internal callbacks.
protected:

	/** Calls optional blueprint implementation of InputReleased. */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** Calls optional blueprint implementation of OnGiveAbility. */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Calls optional blueprint implementation of OnRemoveAbility. */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

// Optional blueprint callbacks.
protected:

	/** Blueprint-implementable event called when this ability's input is completed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName = "Input Released")
	void K2_InputReleased();

	/** Blueprint-implementable event called when this ability is given to an ASC. Called BEFORE C++ OnGiveAbility
	 * super call. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName = "On Give Ability")
	void K2_OnGiveAbility();

	/** Blueprint-implementable event called when this ability is removed from an ASC. Called BEFORE C++ OnRemoveAbility
	 * super call. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName = "On Remove Ability")
	void K2_OnRemoveAbility();



	// Gameplay effects.

protected:

	/** Creates an effect context of type FCrashGameplayEffectContext. */
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;

protected:

	/** Gameplay effects that are applied to the instigating ASC when this ability is activated and persist after the
	 * ability ends. These effects must be removed manually. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", Meta = (ToolTip = "Effects applied when this ability is activated and persist after it ends. Must be removed manually. These should NOT be temporary or one-off effects like \"burning,\" but instead state-based effects like \"Crouching.\""))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStart;

	/** Gameplay effects that are applied to the instigating ASC when this ability is activated and automatically
	* removed when this ability ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", Meta = (ToolTip = "Effects applied when this ability is activated and automatically removed after it ends. These should NOT be temporary or one-off effects like \"burning,\" but instead state-based effects like \"Crouching.\""))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStartAndRemoveOnEnd;

	/** Handles used to track effects applied by this ability that need to be removed when it ends. */
	TArray<FActiveGameplayEffectHandle> EffectsToRemoveOnEndHandles;
};
