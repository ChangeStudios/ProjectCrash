// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/Generic/GA_Death.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/CrashGameplayTags.h"

UGA_Death::UGA_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(CrashGameplayTags::TAG_State_Dying);
	bServerRespectsRemoteAbilityCancellation = false;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	DyingActor = nullptr;
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Checked();
	
	// Cancel ongoing abilities, unless they shouldn't be cancelled by avatar death.
	FGameplayTagContainer IgnoreAbilitiesWithTags;
	IgnoreAbilitiesWithTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_PersistsThroughAvatarDestruction);
	ASC->CancelAbilities(nullptr, &IgnoreAbilitiesWithTags, this);

	// Unpossess the player from the dying actor.
	if (APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		// Cache the dying pawn so we can still reference it.
		DyingActor = Pawn;
		
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
