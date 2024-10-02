// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageExecution.generated.h"

/**
 * Execution used to apply damage to health attributes with gameplay effects. Health attribute values can only be
 * modified via executions and cannot be modified directly.
 *
 * This execution automatically accounts for team damage and self-damage. Effect causers (grenades, weapons, etc.) can
 * implement the EffectSource interface to contextually change the quantity of damage that they deal.
 */
UCLASS()
class PROJECTCRASH_API UDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UDamageExecution();



	// Execution.

protected:

	/** Performs the execution logic. Performs calculations required before applying damage, and then applies damage
	 * to the target's Damage attribute (HealthAttributeSet). */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:

	/** Applies an ultimate charge GE to the instigator of the damage dealt. */
	void GrantUltimateCharge(float DamageApplied, const AActor* Instigator, UAbilitySystemComponent* TargetASC) const;



	// Attribute captures. Any attributes required to perform the execution are captured in the constructor.

protected:

	/** The base damage dealt by this execution, defined by the gameplay effect performing this execution. */
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;
};
