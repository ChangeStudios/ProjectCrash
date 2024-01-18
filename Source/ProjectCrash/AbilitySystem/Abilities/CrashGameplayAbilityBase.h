// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CrashGameplayAbilityBase.generated.h"

class UAbilityTask_WaitInputRelease;
class AChallengerBase;
class UCrashAbilitySystemComponent;

/**
 * Defines how an ability is meant to be activated. Abilities must be instanced-per-actor for these to function
 * properly.
 */
UENUM(BlueprintType)
enum class ECrashAbilityActivationStyle : uint8
{
	/** Try to activate the ability immediately when its input is triggered and do not activate it again until the input
	 * is released. Abilities of this type are automatically ended when the "InputReleased" event is received.
	 * 
	 * Example: Activating a jump when a button is pressed. */
	ActivateOnInputTriggered,

	/** Try to activate the ability once when its input is triggered and cancel it when its input is triggered again,
	 * after the first trigger is released.
	 * TODO: Implement logic that prevents the ability from being activated again before the first input is released.
	 * 
	 * Example: Toggle-based crouching. */
	ToggleOnInputTriggered,

	/** Try to activate the ability once when its input is triggered and cancel it when its input is released.
	 *
	 * Example: Hold-based aiming down sights. */
	ActivateWhileInputHeld,

	/** Try to activate the ability when an avatar is assigned to its ASC.
	 *
	 * Example: Temporary spawn protection. */
	ActivateOnSpawn,

	/** Try to activate the ability when it's granted to an ASC that already has an avatar.
	 *
	 * Example: A passive speed buff. */
	ActivatedOnGranted
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
	FORCEINLINE UCrashAbilitySystemComponent* GetCrashAbilitySystemComponentFromActorInfo() const;

	/** Returns this ability's avatar actor if it is a subclass of the base challenger character class. Returns nullptr
	 * if the avatar could not be found or is of the wrong class. */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Utilities")
	FORCEINLINE AChallengerBase* GetChallengerFromActorInfo() const;



	// Gameplay ability interface.

protected:

	/** Calls optional blueprint implementation of InputReleased. */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	/** Calls optional blueprint implementation of OnGiveAbility. */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	/** Calls optional blueprint implementation of OnRemoveAbility. */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Applies gameplay effects applied by this ability. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	/** Removes gameplay effects applied by this ability that are marked to be removed when it ends. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Creates an effect context of type FCrashGameplayEffectContext. */
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;



	// Tags.

public:

	/** These tags will be bound to corresponding input actions to trigger this ability. These are bound by the owning
	 * ability set when it's granted. */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer InputTags;



	// Optional callback functions.

protected:

	/** Blueprint-implementable event called when this ability's input is completed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName="Input Released")
	void K2_InputReleased();

	/** Blueprint-implementable event called when this ability is given to an ASC. Called BEFORE C++ OnGiveAbility
	 * super call. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName="On Give Ability")
	void K2_OnGiveAbility();

	/** Blueprint-implementable event called when this ability is removed from an ASC. Called BEFORE C++ OnRemoveAbility
	 * super call. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability System|Abilities", DisplayName="On Remove Ability")
	void K2_OnRemoveAbility();



	// Ability activation.

public:

	/** Getter for this ability's activation style. */
	UFUNCTION(BlueprintCallable, Category = "Ability System|Ability Activation")
	ECrashAbilityActivationStyle GetActivationStyle() const { return ActivationStyle; }

protected:

	/** Defines how this ability is meant to be activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation")
	ECrashAbilityActivationStyle ActivationStyle;



	// Ongoing gameplay effects.

protected:

	/** Gameplay effects that are applied to the instigating ASC when this ability is activated and persist after the
	 * ability ends. These effects must be removed manually. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (ToolTip = "Effects applied when this ability is activated and persist after it ends. Must be removed manually. These should NOT be temporary or one-off effects like \"burning,\" but instead state-based effects like \"Crouching.\""))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStart;

	/** Gameplay effects that are applied to the instigating ASC when this ability is activated and automatically
	* removed when this ability ends. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (ToolTip = "Effects applied when this ability is activated and automatically removed after it ends. These should NOT be temporary or one-off effects like \"burning,\" but instead state-based effects like \"Crouching.\""))
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApplyOnStartAndRemoveOnEnd;

	/** Handles used to track effects applied by this ability that need to be removed when it ends. */
	TArray<FActiveGameplayEffectHandle> EffectsToRemoveOnEndHandles;
};
