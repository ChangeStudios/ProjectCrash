// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySlotWidget.generated.h"

class UGameplayAbility;
class UImage;
class UProgressBar;

/**
 * A widget that represents an ability in the HUD. Displays the ability's icon, its cooldown, and whether or not it can
 * be activated.
 *
 * Note that this widget binds to gameplay abilities' CDOs, and will not receive events from ability instances.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UAbilitySlotWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	virtual void NativeConstruct() override;



	// Ability system.

public:

	/** Binds this widget to the given gameplay ability, updating its ability icon and binding its cooldown display and
	 * activation status. */
	UFUNCTION()
	void BindSlotToAbility(UGameplayAbility* Ability);

protected:

	/** The ability to which this slot is currently bound. */
	UPROPERTY(BlueprintReadOnly)
	UGameplayAbility* BoundAbility;



	// UI updates.

protected:

	/** Callback for when this widget's bound ability is activated. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Activated")
	void K2_OnAbilityActivated(UGameplayAbility* Ability);

	/** Callback for when this widget's bound ability ends. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Ability Ended")
	void K2_OnAbilityEnded(UGameplayAbility* Ability);


	/** Callback to update this widget when its bound ability's cooldown is activated. Calculates the cooldown's
	 * duration and passes into K2_OnCooldownStarted. */
	UFUNCTION()
	void OnCooldownStarted(const FActiveGameplayEffect& CooldownGameplayEffect);

	/** Called when this widget's bound ability's cooldown is activated. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Cooldown Started")
	void K2_OnCooldownStarted(const float CooldownDuration);



	// Widgets.

protected:

	/** The icon of the ability that this widget represents. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UImage> AbilityIcon;

	/** The progress bar displaying the ability's current cooldown. Only visible when the ability can be activated and
	 * is on a cooldown. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UProgressBar> CooldownProgressBar;
};
