// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/HealingExecution.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Effects/CrashGameplayEffectContext.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UHealingExecution::UHealingExecution()
{
	// Define the specifications for capturing each attribute needed for this execution.
	BaseHealingDef.AttributeToCapture = UHealthAttributeSet::GetHealingAttribute();
	BaseHealingDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	BaseHealingDef.bSnapshot = true;

	// Capture the attributes needed to perform this execution.
	RelevantAttributesToCapture.Add(BaseHealingDef);
}

void UHealingExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
// Only perform executions on the server.
#if WITH_SERVER_CODE

	// Retrieve this execution's owning gameplay effect.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const TObjectPtr<const UGameplayEffect> OwningGameplayEffect = Spec.Def;


	// Retrieve this execution's gameplay effect context. The owning context contains data that we need to perform the damage execution.
	FCrashGameplayEffectContext* CrashContext = FCrashGameplayEffectContext::GetCrashContextFromHandle(Spec.GetContext());
	if (!CrashContext)
	{
		ABILITY_LOG(Fatal, TEXT("UHealingExecution: Effect context failed to cast to FCrashGameplayEffectContext for healing from [%s]. Context MUST be of type FCrashGameplayEffectContext for healing executions."), *GetPathNameSafe(OwningGameplayEffect));
	}


	// Evaluation parameters for capturing attributes.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;


	// Retrieve information about the source and target.
	const AActor* OriginalInstigator = CrashContext->GetOriginalInstigator();
	const AActor* EffectCauser = CrashContext->GetEffectCauser();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	const AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;


	// Ensure we have the data we need to continue with the execution.
	if (!OriginalInstigator)
	{
		ABILITY_LOG(Error, TEXT("UHealingExecution: No instigator found for healing from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}
	if (!EffectCauser)
	{
		ABILITY_LOG(Error, TEXT("UHealingExecution: No effect causer found for healing from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}
	if (!TargetActor)
	{
		ABILITY_LOG(Fatal, TEXT("UHealingExecution: No target actor found for healing from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}


	// Retrieve the captured base healing value.
	float AmountToHeal;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(BaseHealingDef, EvaluateParameters, AmountToHeal);


	/* Round the healing value down to the nearest whole number. This may be necessary in the future if we deal with
	 * healing multipliers. */
	AmountToHeal = FMath::Floor(AmountToHeal);


	// Apply the healing by adding it to the target's "Healing" attribute.
	if (AmountToHeal > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetHealingAttribute(), EGameplayModOp::Additive, AmountToHeal));
	}

#endif
}
