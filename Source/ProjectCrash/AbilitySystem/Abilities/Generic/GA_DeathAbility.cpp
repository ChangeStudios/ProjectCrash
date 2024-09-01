// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/Generic/GA_DeathAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

UGA_DeathAbility::UGA_DeathAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_DeathAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Checked();
	const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->Controller) : nullptr;

	// Add the "Dying" tag locally.
	ASC->AddLooseGameplayTag(CrashGameplayTags::TAG_State_Dying);

	// Fire the death event delegate through the ASC to broadcast the death data to any listeners.
	if (TriggerEventData)
	{
		// Reconstruct the death data from the given event data.
		const FDeathData DeathData = FDeathData(
			const_cast<AActor*>(TriggerEventData->Target.Get()),
			PC,
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TriggerEventData->Target),
			const_cast<AActor*>(TriggerEventData->Instigator.Get()),
			Cast<AActor>(const_cast<UObject*>(TriggerEventData->OptionalObject2.Get())),
			FGameplayEffectSpec(), // We can't perform a const cast from a USTRUCT, so the effect spec here is invalid.
			TriggerEventData->EventMagnitude);

		// Broadcast the death event delegate.
		if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponentFromActorInfo())
		{
			// CrashASC->DeathEventDelegate.Broadcast(DeathData);
		}
	}

	// Cancel ongoing abilities, unless they shouldn't be cancelled by avatar death.
	FGameplayTagContainer IgnoreAbilitiesWithTags;
	IgnoreAbilitiesWithTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_SurvivesDeath);
	ASC->CancelAbilities(nullptr, &IgnoreAbilitiesWithTags, this);

	// Unpossess the player from the dying actor.
	if (Pawn && Pawn->Controller)
	{
		Pawn->Controller->UnPossess();
	}

	// Make sure we call the K2 implementation after all of this logic is executed (mostly so DyingActor is valid).
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_DeathAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Checked();

	// Remove the local "Dying" tag.
	ASC->RemoveLooseGameplayTag(CrashGameplayTags::TAG_State_Dying);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
