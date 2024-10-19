// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilityWidgetBase.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"

void UAbilityWidgetBase::InitializeWidgetWithAbility(const FGameplayAbilitySpec& AbilitySpec, UCrashAbilitySystemComponent* OwningASC)
{
	ensure(OwningASC);

	// Never re-bind an ability widget.
	ensure(BoundASC == nullptr);

	BoundAbilitySpec = AbilitySpec;
	BoundAbility = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability);
	BoundASC = OwningASC;

	K2_OnAbilityBound();

	// TODO: Call OnActivated if the ability is active, OnDisabled if it's disabled, OnCooldownStarted if it's on
	// cooldown, and OnCostChanged if it has a relevant cost.

	// The Message.Ability tag is implicit, so we need to retrieve it manually.
	const FGameplayTag TAG_Message_Ability = CrashGameplayTags::TAG_Message_Ability_Added.GetTag().RequestDirectParent();

	// Start listening for messages related to the bound ability.
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	AbilityMessageListener = MessageSystem.RegisterListener<FCrashAbilityMessage>(TAG_Message_Ability, [&](FGameplayTag Channel, const FCrashAbilityMessage& Message)
	{
		OnAbilityMessageReceived(TAG_Message_Ability, Message);
	}, EGameplayMessageMatch::PartialMatch);
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
	// Forward ability messages to their corresponding BP events, filtering for messages related to the bound ability.
	if (Message.AbilitySpecHandle == BoundAbilitySpec.Handle &&
		BoundASC.IsValid() &&
		Message.ActorInfo.AbilitySystemComponent == BoundASC.Get())
	{
		if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Disabled))
		{
			K2_OnAbilityDisabled();
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Enabled))
		{
			K2_OnAbilityEnabled();
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Success))
		{
			K2_OnAbilityActivated_Success();
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Ended))
		{
			K2_OnAbilityEnded();
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started))
		{
			K2_OnAbilityCooldownStarted(Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended))
		{
			K2_OnAbilityCooldownEnded();
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_CostChanged))
		{
			K2_OnAbilityCostChanged(Message.Magnitude);
		}
	}
}
