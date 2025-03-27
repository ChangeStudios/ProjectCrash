// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GrantOverhealthExecution.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UGrantOverhealthExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UGrantOverhealthExecution();
	
protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
	
protected:

	/** The base healing dealt by this execution, defined by the gameplay effect performing this execution. */
	FGameplayEffectAttributeCaptureDefinition BaseOverhealthDef;
};
