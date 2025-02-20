// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayAbilitySpec.h"
#include "AbilitySlotWidget.generated.h"

class UCrashActionWidget_Deprecated;
class UCrashAbilitySystemComponent;
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

	// Ability system.

public:

	/** Binds this widget to the given gameplay ability, updating its ability icon and binding its input action. */
	UFUNCTION()
	void BindSlotToAbility(UGameplayAbility* Ability, const UInputAction* InputAction, UCrashAbilitySystemComponent* OwningASC);

protected:

	/** The CDO of the ability to which this slot is currently bound. */
	UPROPERTY(BlueprintReadOnly)
	UGameplayAbility* BoundAbility;

	/** The ASC to which this slot is currently bound. */
	UPROPERTY(BlueprintReadOnly)
	UCrashAbilitySystemComponent* BoundASC;



	// Widgets.

protected:

	/** The icon of the ability that this widget represents. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UImage> AbilityIcon;

	/** The icon of the key bound to the input action to which this ability is bound. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UCrashActionWidget_Deprecated> InputActionWidget;
};
