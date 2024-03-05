// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "UI/Widgets/Utils/AbilitySystemWidget.h"
#include "HealthbarWidget.generated.h"

class UHealthComponent;

/**
 * A widget that binds to its owner's ability system component and receives updates when their Health attribute changes.
 * Uses the owner's pawn's UHealthComponent actor component as an interface to the ASC's health attribute set.
 *
 * This widget requires its owner to be possessing a pawn with a health component.
 */
UCLASS()
class PROJECTCRASH_API UHealthbarWidget : public UAbilitySystemWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Binds callbacks to this widget's owner's health attribute set. */
	virtual void NativeConstruct() override;

protected:

	/** Initializes this widget with the given pawn's health component. If this widget is already bound to a health
	 * component, it will be unbound from that component before binding to the new one. */
	virtual void InitWidget(APawn* PawnToBindTo);

	/** Re-initializes this widget with the new pawn's health component when the widget's owner's pawn changes. */
	UFUNCTION()
	virtual void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** Initializes this widget when its owning player initializes their ASC, if it was not ready when this widget was
	 * created. */
	void OnASCReady();



	// Utils.

protected:

	/** The health component to which this widget is currently bound. */
	UPROPERTY(BlueprintReadOnly, Category = "User Interface|HUD|Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** Handle to the delegate bound to the current health set's OnOutOfHealthDelegate. */
	FDelegateHandle OnOutOfHealthDelegateHandle;



	// Internal attribute update callbacks.

protected:

	/** Callback bound to when this widget's owner's ASC's Health value changes. */
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue);

	/** Callback bound to when this widget's owner's ASC's MaxHealth value changes. */
	UFUNCTION()
	void OnMaxHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue);

	/** Callback bound to when this widget's owner's ASC's Health value reaches 0. */
	UFUNCTION()
	void OnOutOfHealth(UHealthComponent* InHealthComponent, AActor* Instigator, float DamageMagnitude);



	// Optional kismet attribute update callbacks.

protected:

	/** Optional blueprint-exposed function called when this widget's owner's ASC's Health value changes. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (ToolTip = "Called when this widget's owner's ASC's Health value changes."))
	void K2_OnHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue);
	
	/** Optional blueprint-exposed function called when this widget's owner's ASC's MaxHealth value changes. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (ToolTip = "Called when this widget's owner's ASC's MaxHealth value changes."))
	void K2_OnMaxHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue);
	
	/** Optional blueprint-exposed function called when this widget's owner's ASC's Health value reaches 0. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (ToolTip = "Called when this widget's owner's ASC's Health value reaches 0."))
	void K2_OnOutOfHealth(UHealthComponent* InHealthComponent, AActor* Instigator, float DamageMagnitude);
};