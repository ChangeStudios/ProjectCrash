// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Utils/AbilitySystemWidget.h"
#include "GameplayAbilitySpec.h"
#include "AbilityBarWidget.generated.h"

class UAbilitySlotWidget;
class UCrashAbilitySystemComponent;

/**
 * The ability bar is used to display the player's current weapons and abilities, whether they can be activated, and
 * their current cooldown or ammunition.
 *
 * This widget retrieves its owning player's ASC and updates its display automatically. All granted abilities and their
 * current status are displayed according to their UI.AbilityBehavior tags.
 */
UCLASS()
class PROJECTCRASH_API UAbilityBarWidget : public UAbilitySystemWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Initializes this widget with the bound ASC. */
	virtual void OnASCReady() override;



	// UI initialization.

protected:

	/** Initializes this ability bar's display with the current OwningASC. */
	void InitializeAbilityBar();

	/** Adds a new ability widget when an ability is granted to this widget's owner's ASC. */
	virtual void OnAbilityGranted(const FGameplayAbilitySpec& GrantedAbilitySpec) override;

	/** Removes the corresponding ability's widget when it's removed from this widget's owner's ASC. */
	virtual void OnAbilityRemoved(const FGameplayAbilitySpec& RemovedAbilitySpec) override;

	/** Initializes the given ability with the ability bar, creating the necessary widgets for it according to its
	 * UI.AbilityBehavior tags. */
	void InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);

	/** Uninitializes the given ability from the ability bar, removing any widgets that were created for it. */
	void UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);



	// Widget handles.

protected:

	/** The panel to which ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UPanelWidget> AbilitySlotPanel;

	/** The panel to which weapon slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UPanelWidget> WeaponSlotPanel;

	/** References to the current ability slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UCommonActivatableWidget*> AbilitySlotWidgets;

	/** References to the current weapon slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UCommonActivatableWidget*> WeaponSlotWidgets;



	// Data.

protected:

	/** The widget that will be spawned for ability slots in the ability bar. Abilities with the
	 * UI.AbilityBehavior.AbilityBar tag will create their own instance of this widget. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Bar Data")
	TSubclassOf<UAbilitySlotWidget> AbilitySlotWidgetClass;

	/** The widget that will be spawned for weapon slots in the ability bar. Abilities with the
	 * UI.AbilityBehavior.Weapon tag will create their own instance of this widget. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Bar Data")
	TSubclassOf<UCommonActivatableWidget> WeaponSlotWidgetClass;
};