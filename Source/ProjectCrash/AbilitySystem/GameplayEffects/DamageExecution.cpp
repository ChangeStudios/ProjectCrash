// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/GameplayEffects/DamageExecution.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/GameplayEffects/CrashGameplayEffectContext.h"
#include "GameFramework/Teams/TeamSubsystem.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UDamageExecution::UDamageExecution()
{
	// Define the specifications for capturing each attribute needed for this execution.
	BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	DamageBoostDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetDamageBoostAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	DamageResDef = FGameplayEffectAttributeCaptureDefinition(UHealthAttributeSet::GetDamageResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, true);

	// Capture the attributes needed to perform this execution.
	RelevantAttributesToCapture.Add(BaseDamageDef);
	RelevantAttributesToCapture.Add(DamageBoostDef);
	RelevantAttributesToCapture.Add(DamageResDef);
}

void UDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
// Only perform executions on the server.
#if WITH_SERVER_CODE

	// Retrieve this execution's owning spec.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();


	// Retrieve this execution's typed effect context.
	FCrashGameplayEffectContext* CrashContext = FCrashGameplayEffectContext::GetCrashContextFromHandle(Spec.GetContext());
	check(CrashContext);


	// Evaluation parameters for capturing attributes.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;


	// Retrieve the captured damage values.
	float DamageToApply = 0.0f, OutgoingDamageMultiplier = 0.0f, IncomingDamageMultiplier = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(BaseDamageDef, EvaluateParameters, DamageToApply);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageBoostDef, EvaluateParameters, OutgoingDamageMultiplier);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageResDef, EvaluateParameters, IncomingDamageMultiplier);


	// Retrieve information about the source and target.
	const AActor* EffectCauser = CrashContext->GetEffectCauser();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();


	// Calculate data from the context's hit result, if one was given.
	const FHitResult* HitActorResult = CrashContext->GetHitResult();
	AActor* TargetActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	if (HitActorResult)
	{
		const FHitResult& HitResult = *HitActorResult;
		TargetActor = HitResult.HitObjectHandle.FetchActor();
		if (TargetActor)
		{
			ImpactLocation = HitResult.ImpactPoint;
			ImpactNormal = HitResult.ImpactNormal;
			StartTrace = HitResult.TraceStart;
			EndTrace = HitResult.TraceEnd;
		}
	}

	/* If no hit result was given, use the target ASC's avatar as the target actor, and their world location as the
	 * impact location. */
	if (!TargetActor)
	{
		TargetActor = TargetASC ? TargetASC->GetAvatarActor_Direct() : nullptr;
		if (TargetActor)
		{
			ImpactLocation = TargetActor->GetActorLocation();
		}
	}


	// Apply rules for team-damage.
	// NOTE: We already do this when filtering hits with target actors, since it's common for us to want to skip effects
	// besides damage (e.g. knockback) depending on the interaction type. But we do it again here in case we want to
	// consider damage differently. E.g. to make a rocket-jump that applies knockback to self but not damage, we could
	// allow the rocket's explosion to detect its owner, but wouldn't give the effect a "CanDamageSelf" tag.
	float DamageInteractionAllowedMultiplier = 1.0f;
	if (TargetActor)
	{
		UTeamSubsystem* TeamSubsystem = TargetActor->GetWorld()->GetSubsystem<UTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			/* Tags can be added to effect specs to allow damage to self or teammates. Self-destruct damage overrides
			 * any rules. */
			const FGameplayTagContainer& DynamicTags = Spec.GetDynamicAssetTags();
			const bool bCanDamageSelf = DynamicTags.HasTagExact(CrashGameplayTags::TAG_GameplayEffects_Damage_SelfDestruct) ||
										DynamicTags.HasTagExact(CrashGameplayTags::TAG_GameplayEffects_Damage_CanDamageSelf);
			const bool bCanDamageTeam = DynamicTags.HasTagExact(CrashGameplayTags::TAG_GameplayEffects_Damage_CanDamageTeam);

			// NOTE: We disallow self and team damage by default because we very rarely want to damage ourself or our
			// teammates. So when we do, we want to be very explicit and deliberate about it. In other words, players
			// should never damage themselves, but designers have the option to override this if desired.
			// TODO: I think this makes sense, but it's really unintuitive. Should we allow self-damage by default just to prevent potential confusion?
			DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, TargetActor, bCanDamageSelf, bCanDamageTeam) ? 1.0f : 0.0f;
		}
	}


	// Determine distance from causer to target.
	double Distance = WORLD_MAX;

	// Use distance from context origin.
	if (CrashContext->HasOrigin())
	{
		Distance = FVector::Dist(CrashContext->GetOrigin(), ImpactLocation);
	}
	// Use distance from effect causer.
	else if (EffectCauser)
	{
		Distance = FVector::Dist(EffectCauser->GetActorLocation(), ImpactLocation);
	}
	else
	{
		ensureMsgf(false, TEXT("Damage calculation cannot find a source location for damage from [%s]. Falling back to WORLD_MAX..."), *GetPathNameSafe(Spec.Def));
	}


	/* Apply damage falloff. Any other context multipliers (e.g. penetration through physical materials) should be
	 * calculated here too. */
	float DistanceFalloffMultiplier = 1.0f;
	// TODO: Apply damage falloff via EffectSourceInterface on EffectCauser for things like grenades. Implement bp-overridable GetDistanceDropoff function that can be overridden by the grenade projectile.
	DistanceFalloffMultiplier = FMath::Max(DistanceFalloffMultiplier, 0.0f);


	// Calculate and clamp damage.
	DamageToApply = DamageToApply * OutgoingDamageMultiplier * IncomingDamageMultiplier;
	DamageToApply = FMath::Max(DamageToApply * DistanceFalloffMultiplier * DamageInteractionAllowedMultiplier, 0.0f);

	// Round the damage value down to the nearest whole number. We only use whole numbers for health in this project.
	DamageToApply = FMath::Floor(DamageToApply);

	// Apply the damage by adding it to the target's "Damage" attribute.
	if (DamageToApply > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageToApply));
	}

#endif // WITH_SERVER_CODE
}
