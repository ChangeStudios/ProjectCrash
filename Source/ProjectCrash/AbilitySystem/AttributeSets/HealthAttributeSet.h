// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "HealthAttributeSet.generated.h"

struct FCrashVerbMessage;
class UObject;

/**
 * Attribute set for health. Enables an ASC to receive health-related events, such as damage.
 *
 * Health attributes should never be modified directly by gameplay effects. Health should only be modified by
 * executions. Executions enable client-side prediction, take into account the effects of various ongoing systems,
 * and trigger vital functions and delegates.
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UHealthAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UHealthAttributeSet();



	// Attribute changes.

// Execution changes.
protected:

	/** Called before an execution modifies the value of an attribute. Throws out the execution if it attempts to
	 * damage a target with damage invulnerability. */
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

	/** Called after an execution modifies the value of an attribute. Maps changes in meta attribute values to
	 * attribute values. Performs value clamping and broadcasts attribute-change delegates. */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Called before an attribute's base value is modified. Clamps the new base value. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/** Called before an attribute is modified. Clamps the new value. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Called after an attribute is modified. Applies effects of attribute changes, like MaxHealth falling below
	 * Health or Health becoming 0. */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	/** Clamps the given attribute between its minimum and maximum values, depending on the attribute. Rounds the
	 * attribute down to the nearest whole number. */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;



	// Attributes.

// Attribute accessors.
public:

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth);

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Healing);

// Attribute properties.
private:

	/** Current health, defaulted to and capped at MaximumHealth (usually 100). Hidden from modifiers as to only be
	 * modified by executions. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Ability|Attribute|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

		// Tracks when Health reaches 0.
		bool bOutOfHealth;

		// Caches Health before it is updated to determine whether attribute-change delegates should be broadcast.
		float HealthBeforeAttributeChange;

	/** Maximum value that the Health attribute can have at any time. Usually the same as Health's default value. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Ability|Attribute|Health", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;

		// Caches MaxHealth before it is updated to determine whether attribute-change delegates should be broadcast.
		float MaxHealthBeforeAttributeChange;

/* Meta attributes. These attributes are not "stateful"; they are mapped directly to attribute values and then
 * immediately reset after being processed. */
private:

	/** Incoming damage. This is mapped directly to -Health. Positive Damage will remove health. */
	UPROPERTY(BlueprintReadOnly, Category="Ability|Attribute|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Damage;

	/** Incoming healing. This mapped directly to +Health. Positive healing will add health. */
	UPROPERTY(BlueprintReadOnly, Category="Ability|Attribute|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Healing;



	// Attribute delegates. Most variables in these delegates will not be valid on clients.

public:

	/** Delegate broadcast when the Health attribute changes. */
	mutable FAttributeChangedSignature HealthAttributeChangedDelegate;

	/** Delegate broadcast when the MaxHealth attribute changes. */
	mutable FAttributeChangedSignature MaxHealthAttributeChangedDelegate;

	/** Delegate broadcast when the Health attribute reaches 0. */
	mutable FAttributeEventSignature OutOfHealthAttributeDelegate;



	// OnRep functions for attribute changes. Useful for updating things like client UI.

protected:

	/** OnRep for Health. */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	/** OnRep for MaxHealth. */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
};
