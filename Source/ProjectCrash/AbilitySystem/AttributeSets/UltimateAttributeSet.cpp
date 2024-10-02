// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/UltimateAttributeSet.h"

#include "AbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


UUltimateAttributeSet::UUltimateAttributeSet() :
	UltimateCharge(0.0f),
	ChargeRate(1.0f)
{
	InitUltimateCharge(0.0f);
	InitChargeRate(1.0f);
}

void UUltimateAttributeSet::OnDamageMessageReceived(FGameplayTag Channel, const FCrashVerbMessage& Message)
{
	if (IsValid(Message.Instigator) && IsValid(Message.Target))
	{
		// Damage charges ultimates if it's dealt by the player to another player.
		if ((Message.Instigator == GetOwningActor()) && (GetOwningActor() != Message.Target))
		{
			// Ultimate is only charged by damage dealt to player actors, not non-players or minions.
			if (Message.Target.IsA(APlayerState::StaticClass()))
			{
				
			}
		}
	}
}

void UUltimateAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UUltimateAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UUltimateAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
}

void UUltimateAttributeSet::OnRep_UltimateCharge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UUltimateAttributeSet, ChargeRate, OldValue);
}

void UUltimateAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UUltimateAttributeSet, UltimateCharge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UUltimateAttributeSet, ChargeRate, COND_None, REPNOTIFY_Always);
}