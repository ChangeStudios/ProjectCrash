// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "DamageableActor.generated.h"

class UCrashAbilitySystemComponent;
class UHealthAttributeSet;
struct FGameplayEffectContextHandle;

/**
 * Lightweight actor class that can be damaged through the ability system.
 *
 * This class is necessary because target actors won't even process interactions with actors if they don't have an ASC,
 * making lightweight exchanges extremely difficult without a dedicated ability system component.
 */
UCLASS(Abstract, PrioritizeCategories = "Health", Meta = (ToolTip = "Lightweight actor class that can be damaged through the ability system."))
class PROJECTCRASH_API ADamageableActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ADamageableActor();



	// Initialization.

public:

	/** Initializes the ASC and starts listening for damage events. */
	virtual void PostInitializeComponents() override;

	/** Removes damage listeners. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Ability system.

public:

	/** Interface getter for this actor's ASC. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	/**
	 * This actor's internal ability system component. This is simply used as an interface for receiving damage, which
	 * is forwarded to BP as a designer-friendly event.
	 *
	 * Designers should rarely interact with this component directly. The only reason it's even exposed to BP is in case
	 * you want to apply effects to other actors (e.g. a damaging explosive barrel).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Crash|Actor", Meta = (ToolTip = "This actor's ability system component."))
	TObjectPtr<UCrashAbilitySystemComponent> AbilitySystemComponent;

	/** This actor's health attribute set, used to process damage events. */
	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthSet;

	/** How much health this actor has. Will override the health set's initial attribute values and the game mode's
	 * default attribute values to initialize this actor's Health and MaxHealth attributes. */
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float Health;

// BP-exposed functions for damage events. These are the only functions that should be touched on subclasses.
protected:

	/** Triggered locally when this actor takes damage. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Damage Received") 
	void K2_OnDamageReceived(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext);

	/** Triggered locally when this actor runs out of health. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On Death")
	void K2_OnDeath(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext);

/* Health set bindings. Health set events are replicated, so we could bind and calls these on the server and client.
 * But instead, we only call these on the server, and use RPCs to replicate them to clients. This is because, even
 * though the health attribute set is replicated, the event's parameters (e.g. effect contexts) may not be, so we have
 * to manually replicate both the event AND the parameters if we want the client to have them. */
private:

	UFUNCTION()
	void OnDamageReceived(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue);

	UFUNCTION()
	void OnDeath(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float EffectMagnitude);

// RPCs to replicate health event parameters.
private:

	/** Called when this actor loses health on the server. Calls the corresponding blueprint event locally. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDamageReceived(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext);

	/** Called when this actor's health reaches 0 on the server. Calls the corresponding blueprint event locally. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath(UAbilitySystemComponent* InstigatingASC, float DamageAmount, const FGameplayEffectContextHandle& EffectContext);

};
