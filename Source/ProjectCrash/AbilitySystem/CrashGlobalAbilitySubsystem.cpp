// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGlobalAbilitySubsystem.h"

#include "Components/CrashAbilitySystemComponent.h"

/**
 * FGloballyGrantedAbility
 */
void FGloballyGrantedAbility::GrantToASC(UCrashAbilitySystemComponent* ASC)
{
	// If this ability is already granted to the given ASC, remove it before granting it again.
	if (Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	// Grant the ability.
	UGameplayAbility* AbilityCDO = GrantedAbility->GetDefaultObject<UGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);
	
	// Cache the ability's handle.
	Handles.Add(ASC, AbilitySpecHandle);
}

void FGloballyGrantedAbility::RemoveFromASC(UCrashAbilitySystemComponent* ASC)
{
	// Retrieve the global ability's handle for the given ASC.
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		// Remove the ability.
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}



/**
 * UCrashGlobalAbilitySubsystem
 */
void UCrashGlobalAbilitySubsystem::GrantGlobalAbility(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (!GrantedAbilities.Contains(Ability)))
	{
		// Create a new entry to cache the granted ability and its handles.
		FGloballyGrantedAbility& Entry = GrantedAbilities.Add(Ability);
		Entry.GrantedAbility = Ability;

		// Grant the ability to each registered ASC.
		for (UCrashAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.GrantToASC(ASC);
		}
	}
}

void UCrashGlobalAbilitySubsystem::RemoveGlobalAbility(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (GrantedAbilities.Contains(Ability)))
	{
		FGloballyGrantedAbility& Entry = GrantedAbilities[Ability];

		// Remove the ability from each ASC to which it's currently granted.
		for (auto& KVP : Entry.Handles)
		{
			if (KVP.Key != nullptr)
			{
				Entry.RemoveFromASC(KVP.Key);
			}
		}

		// Update the global ability entry.
		GrantedAbilities.Remove(Ability);
	}
}

void UCrashGlobalAbilitySubsystem::RegisterASC(UCrashAbilitySystemComponent* ASC)
{
	check(ASC);

	// Cache the new ASC.
	RegisteredASCs.AddUnique(ASC);

	// Grant each current global ability.
	for (auto& Entry : GrantedAbilities)
	{
		Entry.Value.GrantToASC(ASC);
	}
}

void UCrashGlobalAbilitySubsystem::UnregisterASC(UCrashAbilitySystemComponent* ASC)
{
	check(ASC);

	// Remove each granted global ability.
	if (RegisteredASCs.Find(ASC))
	{
		for (auto& Entry : GrantedAbilities)
		{
			Entry.Value.RemoveFromASC(ASC);
		}
	}

	// Clear the cached ASC.
	RegisteredASCs.Remove(ASC);
}
