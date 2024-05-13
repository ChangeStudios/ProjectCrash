// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/MeleeAttackAbility.h"

#include "../../../../../../../../../Program Files/Epic Games/UE_5.3/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemLog.h"


UMeleeAttackAbility::UMeleeAttackAbility() :
	AttackRange(150.0f),
	AttackRadius(25.0f),
	bDetectEntireDuration(true),
	LastUsed(0.0f),
	CurrentAttackAnim_FPP(0),
	CurrentAttackAnim_TPP(0)
{
}

void UMeleeAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// This ability requires at least one first- and third-person attack montage.
	if (AttackMontages_FPP.Num() == 0 || AttackMontages_TPP.Num() == 0)
	{
#if WITH_EDITOR
		ABILITY_LOG(Error, TEXT("UMeleeAttackAbility: %s: Missing a first- and third-person attack montage. Ability canceled."), *GetName());
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false);
#elif (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		check(AttackMontages_FPP.Num() > 0 && AttackMontages_TPP.Num() > 0)
#endif // WITH_EDITOR
	}

	// There should be an equal amount of first- and third-person attack montages, but this will not break the ability.
	if (AttackMontages_FPP.Num() != AttackMontages_TPP.Num())
	{
		ABILITY_LOG(Warning, TEXT("UMeleeAttackAbility: %s: First- and Third-Person Attack Montages arrays are of different lengths; this may cause mismatched visuals!"), *GetName());
	}

	/*
	 * Find where we'll trace the hit detector from (player camera or actor root)
	 *
	 * Create and configure the target actor
	 *
	 * Reset hit targets
	 *
	 * Reset the attack animation sequence if necessary
	 *
	 * Play the appropriate montages
	 *
	 * if bDetectEntireDuration, start hit detection
	 *
	 * if !bDetectEntireDuration, bind to Event.Ability.MeleeAttack.Start and MeleeAttack.End to start and stop detection
	 *
	 * Update LastUsed
	 *
	 * Update CurrentAttackAnim_FPP and CurrentAttackAnim_TPP
	 */

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
