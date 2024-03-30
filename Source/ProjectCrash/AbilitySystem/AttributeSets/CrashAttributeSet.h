// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CrashAttributeSet.generated.h"

class UCrashAbilitySystemComponent;
struct FGameplayEffectSpec;

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(UHealthSet, Health)
 * ... will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Delegate used to broadcast attribute events, such as running out of health or attribute values changing. Some of
 * these parameters may be null on clients. */
DECLARE_MULTICAST_DELEGATE_FourParams(FAttributeEventSignature, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);
/** Delegate used to broadcast attribute value changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FAttributeChangedSignature, AActor*, EffectInstigator, AActor*, EffectCauser, const FGameplayEffectSpec&, EffectSpec, float, OldValue, float, NewValue);

/**
 * Base class for attribute sets in this project. Handles attribute setup and provides utilities.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	/** Returns the UWorld of this attribute's outer object instead of trying to retrieve it itself. */
	UWorld* GetWorld() const override;

	/** Returns this attribute set's owning ASC, cast to the UCrashAbilitySystemComponent class. Returns nullptr if the
	 * ASC does not exist or is of the wrong class. */
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;
};
