// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/CrashGameplayAbility_Reset.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/CrashCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Player/CrashPlayerState.h"


UCrashGameplayAbility_Reset::UCrashGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// Add a default trigger from the "Reset" event tag.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData DefaultTrigger;
		DefaultTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		DefaultTrigger.TriggerTag = CrashGameplayTags::TAG_GameplayEvent_Player_Reset;
		AbilityTriggers.Add(DefaultTrigger);
	}
}

void UCrashGameplayAbility_Reset::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponentFromActorInfo();
	check(CrashASC);

	SetCanBeCanceled(false);

	/** Cancels abilities as if the character died. */
	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(CrashGameplayTags::TAG_Ability_Behavior_SurvivesDeath);
	CrashASC->CancelAbilities();

	// Reset the character.
	if (ACrashCharacter* CrashChar = GetCrashActorInfo()->CrashCharacter.Get())
	{
		CrashChar->Reset();
	}
	else
	{
		ABILITY_LOG(Error, TEXT("Tried to reset [%s]'s character, but ASC does not have a valid avatar of type CrashCharacter."), *GetNameSafe(ActorInfo->AvatarActor.Get()));
	}

	// Broadcast a message communicating the reset.
	FCrashPlayerResetMessage ResetMessage;
	ResetMessage.OwningActor = ActorInfo->OwnerActor.Get();
	ResetMessage.Controller = GetControllerFromActorInfo();
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_Player_Reset, ResetMessage);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Finish the reset.
	const bool bReplicateEndAbility = true;
	const bool bWasCanceled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCanceled);
}
