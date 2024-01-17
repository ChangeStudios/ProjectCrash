// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/Generic/GA_Jump.h"

#include "AbilitySystemLog.h"
#include "GameFramework/Character.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_Ability_Identifier_Generic_Jump, "Ability.Identifier.Generic.Jump", "Identifier for the generic character jump ability. Includes abilities like double-jump.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag_InputTag_Jump, "InputTag.Jump", "Input tag to activate the jump ability.");

UGA_Jump::UGA_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationStyle = ECrashAbilityActivationStyle::ActivateOnInputTriggered;
}

void UGA_Jump::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);

	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

bool UGA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// We need a valid avatar in order to jump.
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		ABILITY_LOG(Warning, TEXT("[%s] tried to use the [%s] ability, but does not have a valid avatar to perform it. [%s] should not have this ability without a valid avatar."), *GetNameSafe(GetOwningActorFromActorInfo()), *GetName(), *GetNameSafe(GetOwningActorFromActorInfo()));
		return false;
	}

	// Only characters are allowed to jump.
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		ABILITY_LOG(Warning, TEXT("[%s] tried to use the [%s] ability, but its avatar is not a character."), *GetNameSafe(GetOwningActorFromActorInfo()), *GetName());
		return false;
	}

	// Players can only make their local avatar jump.
	if (!Character->IsLocallyControlled())
	{
		return false;
	}

	// The character's CanJump method takes into account jump count, available space, etc.
	return Character->CanJump() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Enable prediction for clients.
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// Commit the ability.
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			return;
		}

		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

		// Use the character's built-in Jump method, integrated with the character movement component.
		ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		Character->Jump();
	}
}

void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping when this ability ends.
	if (ActorInfo && HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
		Character->StopJumping();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
