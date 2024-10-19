// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "AbilityWidgetBase.generated.h"

class UCrashGameplayAbilityBase;
class UCrashAbilitySystemComponent;
struct FCrashAbilityMessage;

/**
 * A widget that receives events related to a specified gameplay ability.
 */
UCLASS()
class PROJECTCRASH_API UAbilityWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Starts listening for ability-related gameplay messages. */
	virtual bool Initialize() override;

	/** Clears gameplay message listeners. */
	virtual void RemoveFromParent() override;



	// Event listening.

private:

	/** Processes the ability message and routes it to BP if relevant. */
	UFUNCTION()
	void OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message);

	/** Listener for the gameplay messages related to this widget's gameplay ability. */
	FGameplayMessageListenerHandle AbilityMessageListener;



	// Blueprint events. These are forwarded to blueprints when their event is received via message.

protected:

	/** This ability was temporarily disabled (not removed). */
	// TODO: Call when the ASC gets the State.AbilityInputBlocked tag.
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Disabled")
	void K2_OnAbilityDisabled(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC);

	/** This ability was re-enabled after being disabled. Not called when the ability is added. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Enabled")
	void K2_OnAbilityEnabled(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC);

	/** This ability was successfully activated by its ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activated")
	void K2_OnAbilityActivated_Success(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC);

	/** This ability ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Ended")
	void K2_OnAbilityEnded(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC);

	/** This ability's cooldown was applied. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Started")
	void K2_OnAbilityCooldownStarted(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC, float CooldownDuration);

	/** This ability's cooldown ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Ended")
	void K2_OnAbilityCooldownEnded(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC);

	/** A variable used as this ability's cost was changed. Magnitude is the variable's new value. E.g. the player's
	 * new ultimate charge. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cost Changed")
	void K2_OnAbilityCostChanged(UCrashGameplayAbilityBase* Ability, UCrashAbilitySystemComponent* ASC, float Magnitude);



	// Internals.

protected:

	/** This widget will only process ability messages for abilities that have this tag. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag AbilityIdentifierTag;
};
