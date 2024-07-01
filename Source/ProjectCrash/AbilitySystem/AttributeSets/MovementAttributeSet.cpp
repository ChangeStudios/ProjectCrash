// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/MovementAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMovementAttributeSet::UMovementAttributeSet() :
	MaxWalkSpeed(600.0f),
	JumpVelocity(1000.0f),
	JumpCount(2.0f)
{
	InitMaxWalkSpeed(600.0f);
	InitJumpVelocity(1000.0f);
	InitJumpCount(2.0f);
}

void UMovementAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UMovementAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UMovementAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UMovementAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
}

void UMovementAttributeSet::OnRep_MaxWalkSpeed(const FGameplayAttributeData& OldValue)
{
}

void UMovementAttributeSet::OnRep_JumpVelocity(const FGameplayAttributeData& OldValue)
{
}

void UMovementAttributeSet::OnRep_JumpCount(const FGameplayAttributeData& OldValue)
{
}

void UMovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MaxWalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, JumpVelocity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, JumpCount, COND_None, REPNOTIFY_Always);
}