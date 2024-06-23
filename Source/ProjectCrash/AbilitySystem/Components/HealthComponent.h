// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthAttributeSet;
class UCrashAbilitySystemComponent;

/** Generic signature for broadcasting health attribute events. E.g. running out of health. */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FHealth_AttributeEventSignature, UHealthComponent* /* HealthComponent */, AActor* /* Instigator */, float /* DamageMagnitude */);
/** Broadcast when an attribute in the Health set changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealth_AttributeChangedSignature, UHealthComponent*, HealthComponent, AActor*, Instigator, float, OldValue, float, NewValue);

/**
 * Component used by actors to interface with health attributes. This compartmentalizes health functionality into a
 * unique component, while also providing an interface to actors that need to be aware of health events, but may not
 * have the health attribute set itself. E.g. the avatar of an ASC that's owned by the player state.
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

	/** Must be called by the owning actor to initialize this component with an ASC and start receiving health events.
	 * The given ASC must have a HealthAttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Health")
	void InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC);

	/** Uninitializes this component from its current ASC. Should be called by the owning actor to stop receiving health
	 * events. Called automatically when this component is unregistered. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Health")
	void UninitializeFromAbilitySystem();

protected:

	/** Automatically uninitializes this component from the ability system when it's unregistered. */
	virtual void OnUnregister() override;



	// Attribute accessors.

public:

	// Returns the current value of the Health attribute.
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Health")
	float GetHealth() const;

	// Returns the current value of the MaxHealth attribute.
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Health")
	float GetMaxHealth() const;

	// Returns the current value of the Health attribute normalized from 0 to MaxHealth.
	UFUNCTION(BlueprintCallable, Category = "Ability|Attribute|Health")
	float GetHealthNormalized() const;



	// Attribute changes.

// Attribute change callbacks.
protected:

	/** Broadcasts HealthChangedDelegate when the Health attribute changes. */
	UFUNCTION()
	void OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Broadcasts MaxHealthChangedDelegate when the MaxHealth attribute changes. */
	UFUNCTION()
	void OnMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Broadcasts OutOfHealthDelegate when the Health attribute reaches 0. */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

/* Attribute change delegates. */
public:

	/** Broadcast when the Health attribute changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature HealthChangedDelegate;

	/** Broadcast when the MaxHealth attribute changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature MaxHealthChangedDelegate;



	// Internals.

protected:

	/** Ability system to which this component is currently bound. */
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** Health attribute set to which this component is currently bound. Owned by AbilitySystemComponent. */
	TObjectPtr<const UHealthAttributeSet> HealthSet;



	// Utils.

public:

	/** Retrieves the given actor's HealthComponent, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute|Health")
	static UHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr); }
};
