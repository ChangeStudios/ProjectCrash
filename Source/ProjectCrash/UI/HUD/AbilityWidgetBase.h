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
 * A widget that receives events related to a bound gameplay ability.
 */
UCLASS()
class PROJECTCRASH_API UAbilityWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Caches the bound ability's info and start listening for ability-related gameplay messages. */
	void InitializeWidgetWithAbility(const FGameplayAbilitySpec& AbilitySpec, UCrashAbilitySystemComponent* OwningASC);

	/** Clears gameplay message listeners. */
	virtual void RemoveFromParent() override;



	// Event listening.

private:

	/** Processes the ability message and routes it to BP if relevant. */
	UFUNCTION()
	void OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message);

	/** Listener for the gameplay messages related to this widget's bound ability. */
	FGameplayMessageListenerHandle AbilityMessageListener;



	// Blueprint events. These are forwarded to blueprints when their event is received via message.

protected:

	/** Called when this widget is bound to a gameplay ability spec on the given ASC. Earliest time as which
	 * BoundAbility and BoundASC are valid. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Bound")
	void K2_OnAbilityBound();

	/** This ability was temporarily disabled (not removed). */
	// TODO: Call when the ASC gets the State.AbilityInputBlocked tag.
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Disabled")
	void K2_OnAbilityDisabled();

	/** This ability was re-enabled after being disabled. Not called when the ability is added. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Enabled")
	void K2_OnAbilityEnabled();

	/** This ability was successfully activated by its ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activated")
	void K2_OnAbilityActivated_Success();

	/** This ability ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Ended")
	void K2_OnAbilityEnded();

	/** This ability's cooldown was applied. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Started")
	void K2_OnAbilityCooldownStarted(float CooldownDuration);

	/** This ability's cooldown ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Ended")
	void K2_OnAbilityCooldownEnded();

	/** A variable used as this ability's cost was changed. Magnitude is the variable's new value. E.g. the player's
	 * new ultimate charge. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cost Changed")
	void K2_OnAbilityCostChanged(float Magnitude);



	// Internals.

protected:

	/** The current spec */
	FGameplayAbilitySpec BoundAbilitySpec;

	/** The CDO of the ability to which this widget is bound. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCrashGameplayAbilityBase> BoundAbility;

	/** The ability system to which this widget is bound: the owner of the bound ability. */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UCrashAbilitySystemComponent> BoundASC;
};
