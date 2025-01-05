// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/HealingExecution.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/GameplayEffects/CrashGameplayEffectContext.h"
#include "GameFramework/Teams/TeamSubsystem.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UHealingExecution::UHealingExecution()
{
	// Define the specifications for capturing each attribute needed for this execution.
	BaseHealingDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetHealingAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);

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


	// Retrieve this execution's typed effect context.
	FCrashGameplayEffectContext* CrashContext = FCrashGameplayEffectContext::GetCrashContextFromHandle(Spec.GetContext());
	check(CrashContext);


	// Evaluation parameters for capturing attributes.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;


	// Retrieve the captured base healing value.
	float HealingToApply = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(BaseHealingDef, EvaluateParameters, HealingToApply);



	// Retrieve information about the source and target.
	const AActor* EffectCauser = CrashContext->GetEffectCauser();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();


	// Try to retrieve the target actor.
	const FHitResult* HitActorResult = CrashContext->GetHitResult();
	AActor* TargetActor = nullptr;

	// Try to get the actor from the hit result.
	if (HitActorResult)
	{
		TargetActor = HitActorResult->HitObjectHandle.FetchActor();
	}

	// Fall back to ASC's avatar.
	if (!TargetActor)
	{
		TargetActor = TargetASC ? TargetASC->GetAvatarActor_Direct() : nullptr;
	}


	// Apply rules for team/opponent healing.
	float HealingInteractionAllowedMultiplier = 0.0f;
	if (TargetActor)
	{
		UTeamSubsystem* TeamSubsystem = TargetActor->GetWorld()->GetSubsystem<UTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			HealingInteractionAllowedMultiplier = TeamSubsystem->CanCauseHealing(EffectCauser, TargetActor) ? 1.0f : 0.0f;
		}
	}


	// Calculate and clamp healing.
	HealingToApply = FMath::Max(HealingToApply * HealingInteractionAllowedMultiplier, 0.0f);

	// Round the healing value DOWN to the nearest whole number. We only use whole numbers for health in this project.
	HealingToApply = FMath::Floor(HealingToApply);

	// Apply the healing by adding it to the target's "Healing" attribute.
	if (HealingToApply > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingToApply));
	}

#endif // WITH_SERVER_CODE
}
