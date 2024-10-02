// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "UltimateAttributeSet.generated.h"

/**
 * Tracks the owning player's current "ultimate ability" charge, and manages the charging of the ability via damage and
 * healing.
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UUltimateAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UUltimateAttributeSet();



	// Charging.

private:

	/** Listener for "Healing" messages. */
	FGameplayMessageListenerHandle HealingListener;

	/** Increases ultimate charge when */
	UFUNCTION()
	void OnDamageMessageReceived(FGameplayTag Channel, const FCrashVerbMessage& Message);



	// Attribute changes.

protected:

	/** Called before an attribute's base value is changed. Clamps the new base value. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/** Called before an attribute is modified. Clamps the new value. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Clamps the given attribute between its minimum and maximum values, depending on the attribute. */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;



	// Attributes.

public:

	ATTRIBUTE_ACCESSORS(UUltimateAttributeSet, UltimateCharge);
	ATTRIBUTE_ACCESSORS(UUltimateAttributeSet, ChargeRate);

private:

	/** The current charge of the player's "ultimate ability," out of 100. The player cannot use their ultimate ability
	 * until their charge is full. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_UltimateCharge, Category = "Ability|Attribute|Abilities", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData UltimateCharge;

	/** Scalar applied when adding to the ultimate charge. */
	UPROPERTY(Replicated)
	FGameplayAttributeData ChargeRate;

protected:

	/** OnRep for UltimateCharge. */
	UFUNCTION()
	void OnRep_UltimateCharge(const FGameplayAttributeData& OldValue);
};
