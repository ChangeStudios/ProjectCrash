// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthAttributeSet;
class UHealthAttributeBaseValues;
class UCrashAbilitySystemComponent;

/** Broadcast when attribute's values are changed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealth_AttributeChanged, UHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

/**
 * Component used by actors with an ASC to interface with health attributes. This compartmentalizes health functionality
 * into a unique component, while also providing an interface to actors that need to be aware of health events, but may
 * not have the health attribute set itself; e.g. the avatar of an ASC that's held a the player state.
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class PROJECTCRASH_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UHealthComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "Ability System|Attributes|Health")
	void InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC, UHealthAttributeBaseValues* InAttributeBaseValues);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Ability System|Attributes|Health")
	void UninitializeFromAbilitySystem();

protected:

	/** Called when this component is unregistered. Wraps UninitializeFromAbilitySystem. */
	virtual void OnUnregister() override;



	// Attribute accessors.

public:

	// Returns the current value of the Health attribute.
	UFUNCTION(BlueprintCallable, Category = "Ability System|Attributes|Health")
	float GetHealth() const;

	// Returns the current value of the MaxHealth attribute.
	UFUNCTION(BlueprintCallable, Category = "Ability System|Attributes|Health")
	float GetMaxHealth() const;

	// Returns the current value of the Health attribute normalized from 0 to MaxHealth.
	UFUNCTION(BlueprintCallable, Category = "Ability System|Attributes|Health")
	float GetHealthNormalized() const;



	// Attribute changes.

// Callbacks bound to when attribute values are changed. These broadcast HealthComponent delegates.
protected:

	/** Called when the Health attribute's value changes. Broadcasts HealthChangedDelegate. */
	UFUNCTION()
	void OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Called when the MaxHealth attribute's value changes. Broadcasts MaxHealthChangedDelegate. */
	UFUNCTION()
	void OnMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Called when Health reaches 0. Broadcasts OutOfHealthDelegate. */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

/* Delegates broadcast when an attribute is changed. These wrap the health set's delegates, provide a reference to the
 * health component, and expose the delegates to BP. */
public:

	/** Delegate broadcast when the Health attribute's value changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChanged HealthChangedDelegate;

	/** Delegate broadcast when the MaxHealth attribute's value changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChanged MaxHealthChangedDelegate;



	// Data.

protected:

	/** The default values to which this set's attributes are initialized. Set when this component is initialized with
	 * an ASC. */
	TSoftObjectPtr<UHealthAttributeBaseValues> AttributeBaseValues;



	// Internal variables.

protected:

	/** Ability system used by this component. */
	UPROPERTY()
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** Health set used by this component. */
	UPROPERTY()
	TObjectPtr<const UHealthAttributeSet> HealthSet;
};
