// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidgetBase.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilityWidgetBase.generated.h"

class UCrashAbilitySystemComponent;
class UCrashGameplayAbilityBase;
struct FCrashAbilityMessage;
struct FGameplayAbilitySpecHandle;

/**
 * A widget that receives events related to a specified gameplay ability. This widget should only be created by the
 * ability to which it will be bound (@see UCrashGameplayAbilityBase::AbilityWidgets).
 */
UCLASS()
class PROJECTCRASH_API UAbilityWidgetBase : public UAbilitySystemWidgetBase
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Starts listening for ability-related gameplay messages. */
	virtual bool Initialize() override;

	/** Clears gameplay message listeners. */
	virtual void RemoveFromParent() override;

	/** Caches the ability spec to which this widget is listening, so we don't have to search for it each time we
	 * receive a message. */
	virtual void OnAbilitySystemBound() override;



	// Event listening.

public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	/** Processes the ability message and routes it to BP if relevant. */
	UFUNCTION()
	void OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message);

	/** Listener for the gameplay messages related to this widget's gameplay ability. */
	FGameplayMessageListenerHandle AbilityMessageListener;



	// Blueprint events. These are forwarded to blueprints when their event is received via message.

protected:

	/** This ability was temporarily disabled (not removed). This can be caused by other ongoing abilities, disabled
	 * input, etc. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Disabled")
	void K2_OnAbilityDisabled(UCrashGameplayAbilityBase* Ability);

	/** This ability was re-enabled after being disabled. Not called when the ability is added. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Enabled")
	void K2_OnAbilityEnabled(UCrashGameplayAbilityBase* Ability);

	/** This ability was successfully activated by its ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activated")
	void K2_OnAbilityActivated_Success(UCrashGameplayAbilityBase* Ability);

	/** This ability ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Ended")
	void K2_OnAbilityEnded(UCrashGameplayAbilityBase* Ability);

	/** This ability's cooldown was applied. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Started")
	void K2_OnAbilityCooldownStarted(UCrashGameplayAbilityBase* Ability, float CooldownDuration);

	/** This ability's cooldown ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Ended")
	void K2_OnAbilityCooldownEnded(UCrashGameplayAbilityBase* Ability);

	/** A variable used as this ability's cost was changed. Magnitude is the variable's new value. E.g. the player's
	 * new ultimate charge. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cost Changed")
	void K2_OnAbilityCostChanged(UCrashGameplayAbilityBase* Ability, float Magnitude);



	// Internals.

protected:

	/** The ability to which this widget should be bound (i.e. the ability whose events this widget should listen
	 * to). */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", DisplayName = "Ability")
	TSoftClassPtr<UCrashGameplayAbilityBase> AbilityClass;

private:

	/** Determines whether to listen for "Ability Disabled" events. Only true if either blueprint event is implemented
	 * to save performance, since listening for these events requires checking the ability's status every tick. */
	bool bWantsDisabledEvents = true;

	/** Tracks the ability's status to prevent "Ability Disabled" events from firing multiple times. */
	bool bIsAbilityEnabled = true;

	/** The ability instance to which this widget is bound. Set when this widget is created, as this widget should
	 * only be created by the ability that it will be bound to (meaning this ability is guaranteed to be valid). */
	UPROPERTY()
	TWeakObjectPtr<UCrashGameplayAbilityBase> BoundAbility;

	/** The spec for the ability instance ot which this widget is bound. */
	UPROPERTY()
	FGameplayAbilitySpecHandle BoundSpecHandle;
};
