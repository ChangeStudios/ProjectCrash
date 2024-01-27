// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "HealingExecution.generated.h"

/**
 * Execution used to apply healing to health attributes with gameplay effects. Health attribute values can only be
 * modified via executions and cannot be modified directly.
 */
UCLASS()
class PROJECTCRASH_API UHealingExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UHealingExecution();



	// Execution.

protected:

	/** Performs the execution logic. Performs calculations required before applying healing, and then applies healing
	 * to the target's Health attributes. */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;



	// Attribute captures. Any attributes required to perform the execution are captured in the constructor.

protected:

	/** The base healing dealt by this execution, defined by the gameplay effect performing this execution. */
	FGameplayEffectAttributeCaptureDefinition BaseHealingDef;
};
