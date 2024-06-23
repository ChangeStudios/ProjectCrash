// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/Generic/GA_Jump.h"

#include "AbilitySystemLog.h"
#include "GameFramework/Character.h"

bool UGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// We need a valid avatar in order to jump.
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		ABILITY_LOG(Warning, TEXT("[%s] tried to activate [%s], but does not have a valid avatar to perform it."), *GetNameSafe(GetOwningActorFromActorInfo()), *GetNameSafe(this));
		return false;
	}

	// Only characters can jump.
	const ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character)
	{
		ABILITY_LOG(Warning, TEXT("[%s] tried to activate [%s], but its avatar [%s] is not a character."), *GetNameSafe(GetOwningActorFromActorInfo()), *GetNameSafe(this), *GetNameSafe(Avatar));
		return false;
	}

	// The character's CanJump method takes into account jump count, available space, etc.
	return Character->CanJump() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActorFromActorInfo());

	// Players can only make their local avatar jump.
	if (Character->IsLocallyControlled())
	{
		// Use the character's built-in Jump method.
		Character->Jump();
	}
}

void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActorFromActorInfo());

	// Players can only make their local avatar jump.
	if (Character->IsLocallyControlled())
	{
		// Stop jumping when this ability ends.
		Character->StopJumping();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
