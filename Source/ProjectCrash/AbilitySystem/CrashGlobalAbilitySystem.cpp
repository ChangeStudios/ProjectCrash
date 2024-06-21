// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGlobalAbilitySystem.h"

#include "Abilities/GameplayAbility.h"
#include "Components/CrashAbilitySystemComponent.h"

void FGlobalGrantedAbilityList::GrantToASC(UCrashAbilitySystemComponent* ASC)
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

void FGlobalGrantedAbilityList::RemoveFromASC(UCrashAbilitySystemComponent* ASC)
{
	// Retrieve the global ability's handle for the given ASC.
	if (FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		// Remove the ability.
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}


UCrashGlobalAbilitySystem::UCrashGlobalAbilitySystem()
{
}

void UCrashGlobalAbilitySystem::GrantGlobalAbility(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (!GrantedAbilities.Contains(Ability)))
	{
		// Create a new entry to cache the granted ability and its handles.
		FGlobalGrantedAbilityList& Entry = GrantedAbilities.Add(Ability);
		Entry.GrantedAbility = Ability;

		// Grant the ability to each registered ASC.
		for (UCrashAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.GrantToASC(ASC);
		}
	}
}

void UCrashGlobalAbilitySystem::RemoveGlobalAbility(TSubclassOf<UGameplayAbility> Ability)
{
	if ((Ability.Get() != nullptr) && (GrantedAbilities.Contains(Ability)))
	{
		// Retrieve the ability list containing the handles of the granted ability.
		FGlobalGrantedAbilityList& Entry = GrantedAbilities[Ability];

		// Remove the ability from each ASC to which it's currently registered.
		for (auto& KVP : Entry.Handles)
		{
			if (KVP.Key != nullptr)
			{
				Entry.RemoveFromASC(KVP.Key);
			}
		}

		// Update the granted abilities list.
		GrantedAbilities.Remove(Ability);
	}
}

void UCrashGlobalAbilitySystem::RegisterASC(UCrashAbilitySystemComponent* ASC)
{
	check(ASC);

	// Grant each current global ability.
	for (auto& Entry : GrantedAbilities)
	{
		Entry.Value.GrantToASC(ASC);
	}

	// Cache the new ASC.
	RegisteredASCs.AddUnique(ASC);
}

void UCrashGlobalAbilitySystem::UnregisterASC(UCrashAbilitySystemComponent* ASC)
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

	// Clear references to the ASC.
	RegisteredASCs.Remove(ASC);
}
