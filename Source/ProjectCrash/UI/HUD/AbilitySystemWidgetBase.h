// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CommonUserWidget.h"
#include "AbilitySystemWidgetBase.generated.h"

class UCrashAbilitySystemComponent;

/**
 * Widget that provides simple access to its owning player's ability system component. Useful for widgets that need
 * to read runtime data from their ASC (e.g. attribute displays).
 */
UCLASS()
class PROJECTCRASH_API UAbilitySystemWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	/** Listens for the owning player's player state to be set so the ASC can be bound. Immediately binds to the owning
	 * player's ASC if it's available. */
	virtual bool Initialize() override;

	/** Stops listening for the player state to be set. */
	virtual void RemoveFromParent() override;

private:

	/** Binds this widget to the player state's ASC. This widget can only be bound once. */
	UFUNCTION()
	void OnPlayerStateChanged(const APlayerState* NewPlayerState);

protected:

	/** Called when this widget is successfully bound to its owning player's ASC. Earliest point at which BoundASC is
	 * readable. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability System Bound")
	void K2_OnAbilitySystemBound();

	/** The owning player's ability system component. Only valid after OnAbilitySystemBound. */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UCrashAbilitySystemComponent> BoundASC;
};
