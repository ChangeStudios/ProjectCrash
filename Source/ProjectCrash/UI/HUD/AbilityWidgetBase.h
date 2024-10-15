// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/TaggedWidget.h"
#include "AbilityWidgetBase.generated.h"

struct FGameplayAbilitySpecHandle;
struct FCrashGameplayAbilityActorInfo;
class UCrashGameplayAbilityBase;
struct FCrashAbilityMessage;
class UCrashAbilitySystemComponent;

/**
 * Widget that receives events related to its owning player's ability system and gameplay abilities.
 */
UCLASS()
class PROJECTCRASH_API UAbilityWidgetBase : public UTaggedWidget
{
	GENERATED_BODY()

	/**
	 *	Initialization:
	 *
	 *		If the PS is available, call OnPSChanged
	 *		If the PS isn't available, bind to when it's set
	 *
	 *		When PS is ready
	 *			Cache ASC
	 *			Iterate through activateable abilities
	 *				Call OnAdded for each
	 *			Start listening for messages with new ASC
	 *
	 *	OnAdded:
	 *		Calls BP OnAdded
	 *		Calls OnActivated if the ability is active
	 *		Calls OnDisabled if the ability is disabled
	 *		Calls OnCooldownStarted if the ability is on cooldown
	 *
	 *	On ASC Bound:
	 *		Listen for tags added to the ASC
	 * 
	 *	Messages:
	 *		OnAbilityAdded
	 *			Fired when an ability is granted to the ASC.
	 *		OnAbilityRemoved
	 *			Fired when an ability is removed from the ASC
	 *		OnAbilityActivated
	 *			Fired when ASC activates an ability
	 *		OnAbilityEnded
	 *			Fired when ASC ends or cancels an ability
	 *		OnAbilityDisabled
	 *			Fired when an ability gets the Ability.Behavior.Disabled tag
	 *			Fired when the ASC gets the State.AbilityInputBlocked tag
	 *		OnAbilityEnabled
	 *			Fired when an ability loses the Ability.Behavior.Disabled tag
	 *			Fired when the ASC loses the State.AbilityInputBlocked tag
	 *		OnAbilityActivationFailed
	 *			Fired when the ASC fails to activate an ability
	 *		OnAbilityCostChanged
	 *			Fired when the cost used for an ability increases or decreases
	 *
	 */

	// Initialization.

public:

	/** Listens for this widget's owning player to set its player state, so we can bind to its ASC. */
	virtual bool Initialize() override;

private:

	/** Attempts to bind to the new player state's ASC. This is only called until an ASC is bound. It will not be called
	 * again if this widget's owning player changes their player state. */
	UFUNCTION()
	void OnPlayerStateChanged(const APlayerState* NewPlayerState);



	// Message listeners.

protected:

	/** Called when any kind of ability message is received. Routes messages to BP events as necessary. */
	UFUNCTION()
	void OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message);

private:

	/** Listener for ability messages. */
	FGameplayMessageListenerHandle AbilityMessageListener;



	// Blueprint events.

// Generic events.
protected:

	/** Called when this widget is bound to its owning player's ASC. Earliest point at which the ASC is accessible. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability System Bound")
	void K2_OnAbilitySystemBound(UCrashAbilitySystemComponent* CrashASC);

	/**
	 * Called when any type of ability message (message in the Message.Ability channel) corresponding to this widget's
	 * ability system is received.
	 *
	 * This is always called IN ADDITION to the message-specific event (e.g. OnAbilityAdded for Message.Ability.Added),
	 * in case you want an ability message that isn't handled by a dedicated event.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Message Received")
	void K2_OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message);

/* Ability events dedicated to each unique ability message to make listening to them easier in BP. Handles are passed to
 * more easily track any data being stored for each ability (e.g. a map of handles to their displayed widget). */
protected:

	/** An ability was granted to this widget's bound ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Added")
	void K2_OnAbilityAdded(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** An ability was removed from this widget's bound ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Removed")
	void K2_OnAbilityRemoved(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** An ability was temporarily disabled (not removed). */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Disabled")
	void K2_OnAbilityDisabled(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** An ability was re-enabled after being disabled. Not called when a usable ability is added. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Enabled")
	void K2_OnAbilityEnabled(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** An ability was activated by this widget's bound ASC. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activated")
	void K2_OnAbilityActivated_Success(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** This widget's bound ASC tried but failed to activate an ability. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activation Failed")
	void K2_OnAbilityActivated_Failed(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** An ability that was activated by this widget's bound ASC ended. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Ended")
	void K2_OnAbilityEnded(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** One of this widget's bound ASC's abilities started a cooldown. Magnitude is the cooldown duration. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Started")
	void K2_OnAbilityCooldownStarted(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** One of this widget's bound ASC's abilities ended its cooldown. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cooldown Ended")
	void K2_OnAbilityCooldownEnded(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);

	/** A variable used for the cost one of this widget's bound ASC's abilities changed. Magnitude is the variable's
	 * new value (e.g. the player's new ultimate charge, or the ability's new charge count. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Cost Changed")
	void K2_OnAbilityCostChanged(FGameplayAbilitySpecHandle AbilityHandle, UCrashGameplayAbilityBase* Ability, const FCrashGameplayAbilityActorInfo ActorInfo, float Magnitude);



	// Internals.

private:

	/** "True" initialization of this widget. Binds listeners to relevant ability system events and messages, and fires
	 * initial events for anything that has already occurred on the ASC (e.g. OnAbilityAdded for abilities that have
	 * already been granted). */
	void OnAbilitySystemBound();

	/** The ability system to which this widget is bound. This ASC's owning controller is this widget's owner. Only set
	 * once, upon initialization, and is never changed. */
	UPROPERTY()
	TWeakObjectPtr<UCrashAbilitySystemComponent> BoundASC;



	// Utils.

protected:

	/** Retrieves the ASC bound to this widget: this widget's owning player's ability system. Will not be usable until
	 * OnAbilitySystemBound is called. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|User Interface|Ability Widget")
	const UCrashAbilitySystemComponent* GetBoundASC() const { return IsValid(BoundASC.Get()) ? BoundASC.Get() : nullptr; }
};
