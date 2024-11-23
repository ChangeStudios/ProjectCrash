// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGlobalAbilitySubsystem.h"

#include "Components/CrashAbilitySystemComponent.h"

/**
 * FGloballyGrantedAbility
 */
void FGloballyGrantedAbility::GrantToASC(TSubclassOf<UGameplayAbility> Ability, UCrashAbilitySystemComponent* ASC)
{
	// If this ability is already granted to the given ASC, remove it before granting it again.
	if (Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	// Grant the ability.
	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
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
 * FGloballyAppliedEffect
 */
void FGloballyAppliedEffect::ApplyToASC(TSubclassOf<UGameplayEffect> Effect, UCrashAbilitySystemComponent* ASC)
{
	// If this effect is already applied to the given ASC, remove it before applying it again.
	if (Handles.Find(ASC))
	{
		RemoveFromASC(ASC);
	}

	// Apply the effect.
	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, 1, ASC->MakeEffectContext());

	// Cache the effect's handle.
	Handles.Add(ASC, GameplayEffectHandle);
}

void FGloballyAppliedEffect::RemoveFromASC(UCrashAbilitySystemComponent* ASC)
{
	// Retrieve the global effect's handle for the given ASC.
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(ASC))
	{
		// Remove the effect.
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
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

		// Grant the ability to each registered ASC.
		for (UCrashAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.GrantToASC(Ability, ASC);
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

		// Clear the global ability entry.
		GrantedAbilities.Remove(Ability);
	}
}

void UCrashGlobalAbilitySubsystem::ApplyGlobalEffect(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && (!AppliedEffects.Contains(Effect)))
	{
		// Create a new entry to cache the applied effect and its handles.
		FGloballyAppliedEffect& Entry = AppliedEffects.Add(Effect);

		// Apply the effect to each registered ASC.
		for (UCrashAbilitySystemComponent* ASC : RegisteredASCs)
		{
			Entry.ApplyToASC(Effect, ASC);
		}
	}
}

void UCrashGlobalAbilitySubsystem::RemoveGlobalEffect(TSubclassOf<UGameplayEffect> Effect)
{
	if ((Effect.Get() != nullptr) && AppliedEffects.Contains(Effect))
	{
		FGloballyAppliedEffect& Entry = AppliedEffects[Effect];

		// Remove the effect from each ASC to which it's currently granted.
		for (auto& KVP : Entry.Handles)
		{
			if (KVP.Key != nullptr)
			{
				KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
			}
		}

		// Clear the global effect entry.
		Entry.Handles.Empty();
		AppliedEffects.Remove(Effect);
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
		Entry.Value.GrantToASC(Entry.Key, ASC);
	}

	// Grant each current global effect.
	for (auto& Entry : AppliedEffects)
	{
		Entry.Value.ApplyToASC(Entry.Key, ASC);
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

		// Remove each current global effect.
		for (auto& Entry : AppliedEffects)
		{
			Entry.Value.RemoveFromASC(ASC);
		}

		// Clear the cached ASC.
		RegisteredASCs.Remove(ASC);
	}
}
