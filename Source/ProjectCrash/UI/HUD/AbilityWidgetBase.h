// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidgetBase.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilityWidgetBase.generated.h"

struct FGameplayAbilitySpecHandle;
class UCrashGameplayAbilityBase;
class UCrashAbilitySystemComponent;
struct FCrashAbilityMessage;

/**
 * A widget that receives events related to a specified gameplay ability.
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

	/** This widget will only process ability messages for abilities that have this tag. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag AbilityIdentifierTag;

private:

	bool bWantsDisabledEvents = true;

	bool bIsAbilityEnabled = true;

	UPROPERTY()
	FGameplayAbilitySpecHandle BoundSpecHandle;
	
	UPROPERTY()
	UCrashGameplayAbilityBase* BoundAbility;



	// Validation.

public:

#if WITH_EDITOR
	/** Displays a warning if bWantsDisabledEvents is true, but the OnAbilityEnabled and OnAbilityDisabled events are
	 * not implemented. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
};
