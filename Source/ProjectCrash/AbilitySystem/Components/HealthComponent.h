// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthAttributeSet;
class UHealthAttributeBaseValues;
class UCrashAbilitySystemComponent;

/** Broadcast on health attribute events, like when the Health attribute reaches 0. */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FHealth_AttributeEventSignature, UHealthComponent* /*HealthComponent*/, AActor* /*Instigator*/, float /*DamageMagnitude*/);
/** Broadcast when health attributes' values are changed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealth_AttributeChangedSignature, UHealthComponent*, HealthComponent, AActor*, Instigator, float, OldValue, float, NewValue);

/**
 * Component used by actors with an ASC to interface with health attributes. This compartmentalizes health functionality
 * into a unique component, while also providing an interface to actors that need to be aware of health events, but may
 * not have the health attribute set itself; e.g. the avatar of an ASC that's held a the player state.
 */
UCLASS(BlueprintType, Meta = (BlueprintSpawnableComponent))
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



	// Utils.

public:

	/** Retrieves the given actor's HealthComponent, if it has one. Otherwise, returns nullptr. */
	UFUNCTION(BlueprintPure, Category = "Ability System|Attributes|Health")
	static UHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr); }



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

	/** Called when Health reaches 0. Broadcasts OutOfHealthDelegate and notifies the game mode of this component's
	 * owning actor's death. */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

/* Delegates broadcast when an attribute is changed. These wrap the health set's delegates, provide a reference to the
 * health component, and expose the delegates to BP. */
public:

	/** Delegate broadcast when the Health attribute's value changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature HealthChangedDelegate;

	/** Delegate broadcast when the MaxHealth attribute's value changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature MaxHealthChangedDelegate;

	/** Delegate broadcast when the Health attribute reaches 0. */
	FHealth_AttributeEventSignature OutOfHealthDelegate;



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
