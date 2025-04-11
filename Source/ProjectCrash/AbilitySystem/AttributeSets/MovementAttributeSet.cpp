// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AttributeSets/MovementAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMovementAttributeSet::UMovementAttributeSet() :
	MaxWalkSpeed(600.0f),
	JumpVelocity(1000.0f),
	JumpCount(2.0f),
	GravityScale(1.5f)
{
	InitMaxWalkSpeed(600.0f);
	InitJumpVelocity(1000.0f);
	InitJumpCount(2.0f);
	InitGravityScale(1.5f);
}

void UMovementAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UMovementAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UMovementAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// Clamp MaxWalkSpeed to safe values.
	if (Attribute == GetMaxWalkSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 150.0f, 1200.0f);
	}
	// Clamp JumpVelocity to safe values.
	else if (Attribute == GetJumpVelocityAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 500.0f, 2000.0f);
	}
	// Clamp JumpCount to safe values. Round it down to the nearest whole number.
	else if (Attribute == GetJumpCountAttribute())
	{
		NewValue = FMath::Clamp(FMath::Floor(NewValue), 1.0f, 10.0f);
	}
	// Clamp GravityScale to safe values.
	else if (Attribute == GetGravityScaleAttribute())
	{
		NewValue = FMath::Clamp(FMath::Floor(NewValue), 0.0f, 3.0f);
	}
}

void UMovementAttributeSet::OnRep_MaxWalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, MaxWalkSpeed, OldValue);
}

void UMovementAttributeSet::OnRep_JumpVelocity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, JumpVelocity, OldValue);
}

void UMovementAttributeSet::OnRep_JumpCount(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, JumpCount, OldValue);
}

void UMovementAttributeSet::OnRep_GravityScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementAttributeSet, GravityScale, OldValue);
}

void UMovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, MaxWalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, JumpVelocity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, JumpCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMovementAttributeSet, GravityScale, COND_None, REPNOTIFY_Always);
}
