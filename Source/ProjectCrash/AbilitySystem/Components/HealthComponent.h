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
/** Signature for events related to death (DeathStarted or DeathFinished). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealth_DeathEventSignature, AActor*, OwningActor);

/**
 * Defines the current state of an actor's death.
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	// Actor is alive.
	NotDead = 0,
	// Actor is currently dying (e.g. ragdolling).
	DeathStarted,
	// Actor is dead (likely pending destruction).
	DeathFinished
};



/**
 * Component used by actors to interface with health attributes. This compartmentalizes health functionality into a
 * unique component, while also providing an interface to actors that need to be aware of health events, but may not
 * have the health attribute set itself. E.g. the avatar of an ASC that's owned by the player state.
 */
UCLASS(BlueprintType, ClassGroup = "Ability System", Meta = (BlueprintSpawnableComponent))
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
	void OnHealthChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Broadcasts MaxHealthChangedDelegate when the MaxHealth attribute changes. */
	UFUNCTION()
	void OnMaxHealthChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	/** Broadcasts OutOfHealthDelegate when the Health attribute reaches 0. */
	UFUNCTION()
	void OnOutOfHealth(AActor* DamageInstigator, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude);

/* Attribute change delegates. */
public:

	/** Broadcast when the Health attribute changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature HealthChangedDelegate;

	/** Broadcast when the MaxHealth attribute changes. */
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChangedSignature MaxHealthChangedDelegate;



	// Death.

public:

	/** Returns the current state of this component's owning actor's death. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute|Health")
	EDeathState GetDeathState() const { return DeathState; }

	/** Whether this component's owning actor is currently dying. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Attribute|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EDeathState::NotDead); }

	/** Starts the death sequence by broadcasting DeathStartedDelegate. */
	void StartDeath();

	/** Finishes the death sequence by broadcasting DeathFinishedDelegate. */
	void FinishDeath();

	/** Fired when the owning actor begins their death sequence. */
	UPROPERTY(BlueprintAssignable)
	FHealth_DeathEventSignature DeathStartedDelegate;

	/** Fired when the owning actor finishes their death sequence, likely before destruction. */
	UPROPERTY(BlueprintAssignable)
	FHealth_DeathEventSignature DeathFinishedDelegate;

private:

	/** The current state of the owning actor's death. */
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDeathState DeathState;

	/** Validates and syncs the death state with the server, which is predictively changed by the Death ability (by
	 * calling StartDeath and FinishDeath locally). */
	UFUNCTION()
	void OnRep_DeathState(EDeathState OldDeathState);



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

	/** Clears the "Dying" and "Dead" tags from the owning ASC. */
	void ClearGameplayTags();
};
