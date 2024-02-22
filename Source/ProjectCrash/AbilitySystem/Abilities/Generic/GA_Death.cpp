// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/Generic/GA_Death.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

UGA_Death::UGA_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(CrashGameplayTags::TAG_State_Dying);
	bServerRespectsRemoteAbilityCancellation = false;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Checked();

	// Fire the death event delegate through the ASC to broadcast the death data to any listeners.
	if (TriggerEventData)
	{
		/* Reconstruct the death data from the given event data. I literally vomited while writing this code but it's
		 * the safest and most efficient way to do this. */
		const FDeathData DeathData = FDeathData(
			const_cast<AActor*>(TriggerEventData->Target.Get()),
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TriggerEventData->Target),
			const_cast<AActor*>(TriggerEventData->Instigator.Get()),
			Cast<AActor>(const_cast<UObject*>(TriggerEventData->OptionalObject2.Get())),
			FGameplayEffectSpec(), // We can't perform a const cast from a USTRUCT, so the effect spec here is invalid.
			TriggerEventData->EventMagnitude);

		// Broadcast the death event delegate.
		if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponentFromActorInfo())
		{
			CrashASC->DeathEventDelegate.Broadcast(DeathData);
		}
	}
	
	// Cancel ongoing abilities, unless they shouldn't be cancelled by avatar death.
	FGameplayTagContainer IgnoreAbilitiesWithTags;
	IgnoreAbilitiesWithTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_PersistsThroughAvatarDestruction);
	ASC->CancelAbilities(nullptr, &IgnoreAbilitiesWithTags, this);

	// Unpossess the player from the dying actor.
	if (APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (Pawn->Controller)
		{
			Pawn->Controller->UnPossess();
		}
	}

	// Make sure we call the K2 implementation after all of this logic is executed (mostly so DyingActor is valid).
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
