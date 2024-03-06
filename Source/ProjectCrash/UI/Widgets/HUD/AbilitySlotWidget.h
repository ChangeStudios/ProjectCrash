// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AbilitySlotWidget.generated.h"

class UGameplayAbility;
class UImage;
class UProgressBar;

/**
 * A widget that represents an ability in the HUD. Displays the ability's icon, its cooldown, and whether or not it can
 * be activated.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UAbilitySlotWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	virtual void NativeConstruct() override;

	/** Binds this widget to the given gameplay ability, updating its ability icon and binding its cooldown display and
	 * activation status. */
	UFUNCTION()
	void BindSlotToAbility(UGameplayAbility* Ability);

protected:

	/** The ability to which this slot is currently bound. */
	UPROPERTY()
	UGameplayAbility* BoundAbility;



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
