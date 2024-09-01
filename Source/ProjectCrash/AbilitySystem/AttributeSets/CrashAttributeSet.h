// Copyright Samuel Reitich. All rights reserved.

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
 *
 * Yes, this is a macro of macros. Deal with it.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Delegate used to broadcast attribute events, such as running out of health. Some of these parameters may be null on
 * clients. */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAttributeEventSignature, AActor* /* EffectInstigator */, const FGameplayEffectSpec& /* EffectSpec */, float /* EffectMagnitude */);
/** Delegate used to broadcast attribute value changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAttributeChangedSignature, AActor*, EffectInstigator, const FGameplayEffectSpec&, EffectSpec, float, OldValue, float, NewValue);

/**
 * Base class for attribute sets in this project. Provides utilities and helpers for defining common attribute
 * functions.
 *
 * Note the importance of calling the ATTRIBUTE_ACCESSORS for each attribute you define when subclassing this. This is
 * not required but is very helpful.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	// Utils.

public:

	/** Returns the UWorld of this attribute's outer object (usually the ASC) instead of trying to retrieve it
	 * itself. */
	UWorld* GetWorld() const override;

	/** Returns this attribute set's owning ASC as a CrashAbilitySystemComponent. Returns null if the ASC does not
	 * exist or is of the wrong type. */
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;
};
