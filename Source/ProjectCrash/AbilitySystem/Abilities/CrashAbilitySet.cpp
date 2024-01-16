// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/CrashAbilitySet.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

void FCrashAbilitySet_GrantedHandles::AddGameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& HandleToAdd)
{
	// Store the ability handle if it is valid.
	if (HandleToAdd.IsValid())
	{
		GrantedAbilitySpecHandles.Add(HandleToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& HandleToAdd)
{
	// Store the effect handle if it is valid.
	if (HandleToAdd.IsValid())
	{
		AppliedEffectHandles.Add(HandleToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* SetToAdd)
{
	// Store the attribute set pointer if it is valid.
	if (SetToAdd != nullptr)
	{
		GrantedAttributeSets.Add(SetToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::RemoveFromAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToRemoveFrom)
{
	check(AbilitySystemToRemoveFrom);

	// Authority is needed to give or remove ability sets.
	if (!AbilitySystemToRemoveFrom->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Remove each ability granted by this ability set.
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemToRemoveFrom->ClearAbility(Handle);
		}
	}

	// Remove each effect applied by this ability set.
	for (const FActiveGameplayEffectHandle& Handle : AppliedEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemToRemoveFrom->RemoveActiveGameplayEffect(Handle);
		}
	}

	// Remove each attribute set given by this ability set.
	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		if (Set != nullptr)
		{
			AbilitySystemToRemoveFrom->RemoveSpawnedAttribute(Set);
		}
	}

	// Clear the handle collections.
	GrantedAbilitySpecHandles.Reset();
	AppliedEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

void UCrashAbilitySet::GiveToAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToGiveTo,
                                           FCrashAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
}
