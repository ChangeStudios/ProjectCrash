// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilityWidgetBase.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "GameFramework/PlayerState.h"

bool UAbilityWidgetBase::Initialize()
{
	if (!IsDesignTime())
	{
		// The Message.Ability tag is implicit, so we need to retrieve it manually.
		const FGameplayTag TAG_Message_Ability = CrashGameplayTags::TAG_Message_Ability_Added.GetTag().RequestDirectParent();

		// Start listening for messages related to the bound ability.
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		AbilityMessageListener = MessageSystem.RegisterListener<FCrashAbilityMessage>(TAG_Message_Ability, [&](FGameplayTag Channel, const FCrashAbilityMessage& Message)
		{
			OnAbilityMessageReceived(TAG_Message_Ability, Message);
		}, EGameplayMessageMatch::PartialMatch);
	}

	return Super::Initialize();
}

void UAbilityWidgetBase::RemoveFromParent()
{
	if (AbilityMessageListener.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(World);
			MessageSystem.UnregisterListener(AbilityMessageListener); 
		}
	}

	Super::RemoveFromParent();
}

void UAbilityWidgetBase::OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message)
{
	// We only care about OUR abilities.
	if (!Message.ActorInfo.OwnerActor.IsValid() ||
		 Message.ActorInfo.OwnerActor.Get() != GetOwningPlayerState())
	{
		return;
	}

	if (!ensure(BoundASC.IsValid()))
	{
		return;
	}

	if (!ensure(Message.AbilitySpecHandle.IsValid()))
	{
		return;
	}

	const FGameplayAbilitySpec* AbilitySpec = BoundASC->FindAbilitySpecFromHandle(Message.AbilitySpecHandle);
	UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
	if (!IsValid(Ability))
	{
		Ability = AbilitySpec->Ability;
	}
	UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability);

	// Forward ability messages to their corresponding events, filtering for messages related to the specified ability.
	if (CrashAbility->AbilityTags.HasTag(AbilityIdentifierTag))
	{
		if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Disabled))
		{
			K2_OnAbilityDisabled(CrashAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Enabled))
		{
			K2_OnAbilityEnabled(CrashAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Success))
		{
			K2_OnAbilityActivated_Success(CrashAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Ended))
		{
			K2_OnAbilityEnded(CrashAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started))
		{
			K2_OnAbilityCooldownStarted(CrashAbility, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended))
		{
			K2_OnAbilityCooldownEnded(CrashAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_CostChanged))
		{
			K2_OnAbilityCostChanged(CrashAbility, Message.Magnitude);
		}
	}
}
