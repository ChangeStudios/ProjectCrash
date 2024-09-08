// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/Generic/CrashGameplayAbility_Death.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/HealthComponent.h"

UCrashGameplayAbility_Death::UCrashGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add a default trigger from the "Death" event tag.
		FAbilityTriggerData DefaultTrigger;
		DefaultTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		DefaultTrigger.TriggerTag = CrashGameplayTags::TAG_GameplayEvent_Ability_Death;
		AbilityTriggers.Add(DefaultTrigger);

		// Add a "Dying" tag to the dying ASC while this ability is active.
		ActivationOwnedTags.AddTag(CrashGameplayTags::TAG_State_Dying);
	}

	// This ability cannot be canceled and should always be blocking.
	UGameplayAbility::SetCanBeCanceled(false);
	ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;
}

void UCrashGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	// Cancel all abilities without the "SurvivesDeath" tag.
	FGameplayTagContainer SurvivesDeath = FGameplayTagContainer(CrashGameplayTags::TAG_Ability_Behavior_SurvivesDeath);
	ActorInfo->AbilitySystemComponent.Get()->CancelAbilities(nullptr, &SurvivesDeath, this);

	// Start the actor's death sequence through their health component.
	if (UHealthComponent* HealthComponent = UHealthComponent::FindHealthComponent(ActorInfo->AvatarActor.Get()))
	{
		HealthComponent->StartDeath();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UCrashGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	check(ActorInfo);

	// Finish the actor's death sequence through their health component.
	if (UHealthComponent* HealthComponent = UHealthComponent::FindHealthComponent(ActorInfo->AvatarActor.Get()))
	{
		HealthComponent->FinishDeath();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

