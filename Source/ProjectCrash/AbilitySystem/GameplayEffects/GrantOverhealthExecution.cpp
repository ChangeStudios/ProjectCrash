// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/GrantOverhealthExecution.h"

#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"


UGrantOverhealthExecution::UGrantOverhealthExecution()
{
	// Define the specifications for capturing each attribute needed for this execution.
	BaseOverhealthDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetOverhealthAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);

	// Capture the attributes needed to perform this execution.
	RelevantAttributesToCapture.Add(BaseOverhealthDef);
}

void UGrantOverhealthExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Retrieve this execution's owning gameplay effect.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	// Retrieve the captured base healing value.
	float HealingToApply = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(BaseOverhealthDef, EvaluateParameters, HealingToApply);

	UE_LOG(LogTemp, Error, TEXT("Healing: %f"), HealingToApply);
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetOverhealthAttribute(), EGameplayModOp::Additive, HealingToApply));
}
