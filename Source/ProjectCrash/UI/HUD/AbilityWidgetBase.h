// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AbilityWidgetBase.generated.h"

class UCrashAbilitySystemComponent;

/**
 * Widget that receives events related to its owning player's ability system and gameplay abilities.
 */
UCLASS()
class PROJECTCRASH_API UAbilityWidgetBase : public UCommonUserWidget
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



	// Blueprint events.

protected:

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability System Bound")
	void K2_OnAbilitySystemBound(UCrashAbilitySystemComponent* CrashASC);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Added")
	void K2_OnAbilityAdded();



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
};
