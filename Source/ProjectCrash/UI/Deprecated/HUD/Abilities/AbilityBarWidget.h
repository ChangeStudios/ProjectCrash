// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "UI/Deprecated/AbilitySystemWidget.h"
#include "AbilityBarWidget.generated.h"

class UAbilitySlotWidget;
class UDynamicEntryBox;
class UWeaponSlotWidget;

/**
 * The ability bar is used to display the player's current Challenger and equipment abilities, and their current
 * information, such as their current cooldown or ammunition.
 */
UCLASS()
class PROJECTCRASH_API UAbilityBarWidget : public UAbilitySystemWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	// Initializes this widget with its owner's ASC and binds to relevant events in the ASC.
	virtual void OnASCReady() override;



	// ASC events.

protected:

	/** Re-activates this widget when its owner's ASC is initialized (e.g. when its owning player respawns). */
	UFUNCTION()
	void OnASCInit(AActor* OwnerActor, AActor* AvatarActor);

	/** Deactivates this widget when its owning player dies. */
	UFUNCTION()
	void OnDeath();



	// Abilities.

protected:

	/** Initializes the given ability with the ability bar, creating the necessary widgets for it according to its
	 * UI.AbilityBehavior tags. */
	UFUNCTION()
	void InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);

	/** Uninitializes the given ability from the ability bar, removing any widgets that were created for it. */
	UFUNCTION()
	void UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);



	// Widgets.

// Widget containers.
protected:

	/** The widget to which ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> AbilitySlotBox;

	/** The widget to which equipment ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EquipmentSlotBox;

	/** The widget to which weapon ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> WeaponSlotBox;

// Cached widgets.
protected:

	/** References to the current ability slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UAbilitySlotWidget*> AbilitySlotWidgets;

	/** References to the current equipment slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UAbilitySlotWidget*> EquipmentSlotWidgets;

	/** References to the current weapon slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UWeaponSlotWidget*> WeaponSlotWidgets;

// Utils.
protected:

	void SortAbilityWidgets(UDynamicEntryBox* BoxToSort);

	FTimerHandle TimerHandle;
};