// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/DamageExecution.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Effects/CrashGameplayEffectContext.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UDamageExecution::UDamageExecution()
{
	// Define the specifications for capturing each attribute needed for this execution.
	BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);

	// Capture the attributes needed to perform this execution.
	RelevantAttributesToCapture.Add(BaseDamageDef);
}

void UDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
// Only perform executions on the server.
// TODO: Finish this function.
#if WITH_SERVER_CODE

	// Retrieve this execution's owning gameplay effect.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const TObjectPtr<const UGameplayEffect> OwningGameplayEffect = Spec.Def;


	// Retrieve this execution's gameplay effect context. The owning context contains data that we need to perform the damage execution.
	FCrashGameplayEffectContext* CrashContext = FCrashGameplayEffectContext::GetCrashContextFromHandle(Spec.GetContext());
	if (!CrashContext)
	{
		ABILITY_LOG(Fatal, TEXT("UDamageExecution: Effect context failed to cast to FCrashGameplayEffectContext for damage from [%s]. Context must be of type FCrashGameplayEffectContext for damage executions."), *GetPathNameSafe(OwningGameplayEffect));
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
		ABILITY_LOG(Error, TEXT("UDamageExecution: No instigator found for damage from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}
	if (!EffectCauser)
	{
		ABILITY_LOG(Error, TEXT("UDamageExecution: No effect causer found for damage from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}
	if (!TargetActor)
	{
		ABILITY_LOG(Fatal, TEXT("UDamageExecution: No target actor found for damage from [%s]."), *GetPathNameSafe(OwningGameplayEffect))
	}


	// If this damage execution is targeting the actor that caused it, check if it can damage its source.
	if (TargetActor == OriginalInstigator || TargetActor == EffectCauser)
	{
		if (const UGameplayEffectComponent* AssetTagComp = OwningGameplayEffect->FindComponent(UAssetTagsGameplayEffectComponent::StaticClass()))
		{
			// If this effect doesn't have the "CanDamageSelf" tag but is targeting its source, throw out this execution.
			if (!Cast<UAssetTagsGameplayEffectComponent>(AssetTagComp)->GetConfiguredAssetTagChanges().CombinedTags.HasTagExact(CrashGameplayTags::TAG_Effects_Damage_CanDamageSelf))
			{
				return;
			}
		}
	}

	// TODO: Add a CanDamage function to check for team damage too.


	// Retrieve the captured base damage value.
	float DamageToApply;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(BaseDamageDef, EvaluateParameters, DamageToApply);

	/* Round the damage value down to the nearest whole number. This may be necessary in the future if we deal with
	 * damage multipliers. */
	DamageToApply = FMath::Floor(DamageToApply);

	// Apply the damage by adding it to the target's "Damage" attribute.
	if (DamageToApply > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageToApply));
	}

#endif
}
