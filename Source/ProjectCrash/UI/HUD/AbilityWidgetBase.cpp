// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilityWidgetBase.h"

#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Player/CrashPlayerController.h"

bool UAbilityWidgetBase::Initialize()
{
	if (!IsDesignTime())
	{
		ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>();
		if (ensure(CrashPC))
		{
			// If the player state is ready, immediately bind to it.
			if (APlayerState* PS = CrashPC->GetPlayerState<APlayerState>())
			{
				OnPlayerStateChanged(PS);
			}
			// If the player state hasn't been set yet, wait for it before attempting to bind.
			else
			{
				CrashPC->PlayerStateChangedDelegate.AddDynamic(this, &UAbilityWidgetBase::OnPlayerStateChanged);
			}
		}
	}

	return Super::Initialize();
}

void UAbilityWidgetBase::OnPlayerStateChanged(const APlayerState* NewPlayerState)
{
	// When the first valid player state is received, bind to its ASC.
	if (NewPlayerState && !BoundASC.Get())
	{
		BoundASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(NewPlayerState);
		ensureAlwaysMsgf(BoundASC.Get(), TEXT("Attempted to bind an ability widget [%s] to its owner [%s]'s ASC, but an ASC could not be found."), *GetPathNameSafe(this), *GetNameSafe(NewPlayerState));

		// Only bind to the first player state the owning controller gets.
		if (ACrashPlayerController* CrashPC = GetOwningPlayer<ACrashPlayerController>())
		{
			CrashPC->PlayerStateChangedDelegate.RemoveAll(this);
		}

		// Proper initialization.
		OnAbilitySystemBound();
	}
}

void UAbilityWidgetBase::OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message)
{
	// Forward ability messages (filtering for this widget's ASC) to their corresponding BP events.
	if (ensure(BoundASC.Get()) && (Message.ActorInfo.AbilitySystemComponent == BoundASC.Get()))
	{
		// Specs aren't well-exposed to BP, so just pass the CDO instead.
		const FGameplayAbilitySpec* AbilitySpec = BoundASC->FindAbilitySpecFromHandle(Message.AbilitySpecHandle);
		UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
		if (!IsValid(Ability))
		{
			Ability = AbilitySpec->Ability;
		}
		UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability);

		// TODO: C++ please add switches for structs thank you <3
		if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Added))
		{
			K2_OnAbilityAdded(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Removed))
		{
			K2_OnAbilityRemoved(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Success))
		{
			K2_OnAbilityActivated_Success(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Failed))
		{
			K2_OnAbilityActivated_Failed(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Ended))
		{
			K2_OnAbilityEnded(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started))
		{
			K2_OnAbilityCooldownStarted(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended))
		{
			K2_OnAbilityCooldownEnded(Message.AbilitySpecHandle, CrashAbility, Message.ActorInfo, Message.Magnitude);
		}

		K2_OnAbilityMessageReceived(Message.MessageType, Message);
	}
}

void UAbilityWidgetBase::OnAbilitySystemBound()
{
	ensure(BoundASC.Get());

	// Call OnAbilityAdded for abilities that we already have (abilities granted before this widget was created).
	for (FGameplayAbilitySpec& Spec : BoundASC.Get()->GetActivatableAbilities())
	{
		UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Spec.Ability);
		K2_OnAbilityAdded(Spec.Handle, CrashAbility, *BoundASC.Get()->GetCrashAbilityActorInfo(), Spec.Level);
	}

	// The Message.Ability tag is implicit, so we need to retrieve it manually.
	const FGameplayTag GenericAbilityTag = CrashGameplayTags::TAG_Message_Ability_Added.GetTag().RequestDirectParent();

	// Start listening for any ability messages.
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	AbilityMessageListener = MessageSystem.RegisterListener<FCrashAbilityMessage>(GenericAbilityTag, [&](FGameplayTag Channel, const FCrashAbilityMessage& Message)
	{
		OnAbilityMessageReceived(GenericAbilityTag, Message);
	}, EGameplayMessageMatch::PartialMatch);

	K2_OnAbilitySystemBound(BoundASC.Get());
}
