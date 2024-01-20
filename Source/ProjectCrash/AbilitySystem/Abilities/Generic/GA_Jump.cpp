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

	// The character's CanJump method takes into account jump count, available space, etc.
	return Character->CanJump() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());

	// Players can only make their local avatar jump.
	if (Character->IsLocallyControlled())
	{
		// Use the character's built-in Jump method, integrated with the character movement component.
		Character->Jump();
	}
}

void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());

	// Players can only make their local avatar jump.
	if (Character->IsLocallyControlled())
	{
		// Stop jumping when this ability ends.
		Character->StopJumping();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
