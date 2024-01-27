// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageExecution.generated.h"

/**
 * Execution used to apply damage to health attributes with gameplay effects. Health attribute values can only be
 * modified via executions and cannot be modified directly.
 *
 * This execution automatically accounts for team damage, self-damage, and can be extended to account for any future
 * mechanics that alter damage, such as damage drop-off or damage boosts.
 */
UCLASS(Blueprintable, Const)
class PROJECTCRASH_API UDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UDamageExecution();



	// Execution.

protected:

	/** Performs the execution logic. Any calculations required before applying damage are done, and then damage is
	 * applied to the target attributes. */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;



	// Attribute captures. Any attributes required to perform the execution are captured in the constructor.

protected:

	/** The base damage dealt by this execution, defined by the gameplay effect performing this execution. */
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

};
