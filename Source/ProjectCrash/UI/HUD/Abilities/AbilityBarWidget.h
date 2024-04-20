// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "UI/Widgets/Utils/AbilitySystemWidget.h"
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

	virtual void OnASCReady() override;



	// ASC events.

protected:

	UFUNCTION()
	void OnASCInit(AActor* OwnerActor, AActor* AvatarActor);

	UFUNCTION()
	void OnDeath(const FDeathData& DeathData);



	// Abilities.

protected:

	/** Initializes the given ability with the ability bar, creating the necessary widgets for it according to its
	 * UI.AbilityBehavior tags. */
	UFUNCTION()
	void InitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);

	/** Uninitializes the given ability from the ability bar, removing any widgets that were created for it. */
	UFUNCTION()
	void UninitializeAbilityWithUI(const FGameplayAbilitySpec& AbilitySpec);



	// Widget handles.

protected:

	/** The widget to which ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> AbilitySlotPanel;

	/** The widget to which equipment ability slots will be added. */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EquipmentSlotPanel;

	/** References to the current ability slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UCommonActivatableWidget*> AbilitySlotWidgets;

	/** References to the current weapon slot widgets that have been created for abilities. */
	UPROPERTY()
	TMap<UGameplayAbility*, UCommonActivatableWidget*> WeaponSlotWidgets;
};