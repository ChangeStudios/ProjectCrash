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

	/** Starts listening for damage or healing dealt by the owning player, to automatically grant ultimate charge. */
	virtual void PostInitProperties() override;



	/* Charging. Ultimates can be charged by other means (e.g. a pick-up), but we automatically handle damage and
	 * healing here. We could do this in the damage/healing executions, but I prefer compartmentalizing all the ultimate
	 * logic into this attribute set. */

private:

	/** Listener for "Damage" messages. */
	FGameplayMessageListenerHandle DamageListener;

	/** Listener for "Healing" messages. */
	FGameplayMessageListenerHandle HealingListener;

	/** Grants ultimate charge when the owning player deals damage or healing. */
	UFUNCTION()
	void GrantUltimateChargeFromEffect(FGameplayTag Channel, const FCrashVerbMessage& Message);

	/** Determines whether an instigator should gain ultimate charge for damaging a target. Returns true if the target
	 * is another player and on a different team, if any. */
	bool ShouldDamageGrantUltimateCharge(AActor* Instigator, AActor* Target) const;

	/** Determines whether an instigator should gain ultimate charge for healing a target. Returns true if the target
	 * is a player and on a different team, if any. This is the same as damage, except that ultimate charge IS granted
	 * for players healing themselves. */
	bool ShouldHealingGrantUltimateCharge(AActor* Instigator, AActor* Target) const;



	// Attribute changes.

protected:

	/** Called before an attribute's base value is changed. Clamps the new base value. */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	/** Called before an attribute is modified. Clamps the new value. */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Called after modifying the value of an attribute on the server. Broadcasts the CostChanged message on the
	 * server.*/
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/** Clamps the ultimate charge attribute between its minimum and maximum values. Scales the ultimate charge with
	 * the "UltimateChargeRate" game mode property. */
	void ClampAndScaleUltimateCharge(const FGameplayAttribute& Attribute, float& NewValue) const;



	// Attributes.

public:

	ATTRIBUTE_ACCESSORS(UUltimateAttributeSet, UltimateCharge);

private:

	/** The current charge of the player's "ultimate ability." The player cannot use their ultimate ability until their
	 * charge has reached the cost of their ultimate. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_UltimateCharge, Category = "Ability|Attribute|Abilities", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData UltimateCharge;

protected:

	/** OnRep for UltimateCharge. Broadcasts the CostChanged message on clients. */
	UFUNCTION()
	void OnRep_UltimateCharge(const FGameplayAttributeData& OldValue);



	// Utils.

public:

	/** Returns the ultimate charge required for the owning player's ultimate ability. */
	UFUNCTION(BlueprintPure, Category = "Ability|Attribute|Abilities")
	float GetMaxUltimateCharge() const;

protected:

	/**
	 * Finds the owning player's ultimate ability. This is the first (and should be the only) gameplay ability that uses
	 * a gameplay effect cost requiring the UltimateCharge attribute.
	 *
	 * @param OutUltimateAbility	The ultimate ability, if one exists.
	 * @param CostMagnitude			The magnitude of the UltimateCharge attribute required by the ultimate ability's
	 *								cost, if an ultimate ability exists.
	 *
	 * @return						Whether an ability was found.
	 */
	bool GetUltimateAbility(FGameplayAbilitySpec& OutUltimateAbility, float& CostMagnitude) const;

	/** Broadcasts an Ability.CostChanged message using the current value of UltimateCharge as the magnitude. */
	void BroadcastUltChargeChanged() const;
};
