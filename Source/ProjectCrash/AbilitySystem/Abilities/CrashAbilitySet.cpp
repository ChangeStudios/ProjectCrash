// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/CrashAbilitySet.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayAbilityBase.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

/**
 * FCrashAbilitySet_GrantedHandles
 */
void FCrashAbilitySet_GrantedHandles::AddGameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& HandleToAdd)
{
	// Cache the ability handle if it is valid.
	if (HandleToAdd.IsValid())
	{
		GrantedAbilitySpecHandles.Add(HandleToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& HandleToAdd)
{
	// Cache the effect handle if it's valid.
	if (HandleToAdd.IsValid())
	{
		AppliedEffectHandles.Add(HandleToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* SetToAdd)
{
	// Cache the attribute set pointer if it's valid.
	if (SetToAdd != nullptr)
	{
		AddedAttributeSets.Add(SetToAdd);
	}
}

void FCrashAbilitySet_GrantedHandles::RemoveFromAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToRemoveFrom)
{
	check(AbilitySystemToRemoveFrom);

	// Ability sets can only be removed by the server.
	if (!AbilitySystemToRemoveFrom->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Remove each ability granted by this ability set.
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			// Remove the ability.
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

	// Remove each attribute set granted by this ability set.
	for (UAttributeSet* Set : AddedAttributeSets)
	{
		if (Set)
		{
			AbilitySystemToRemoveFrom->RemoveSpawnedAttribute(Set);
		}
	}

	// Clear the handle collections.
	GrantedAbilitySpecHandles.Reset();
	AppliedEffectHandles.Reset();
	AddedAttributeSets.Reset();
}



/**
 * UCrashAbilitySet
 */
void UCrashAbilitySet::GiveToAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToGiveTo, FCrashAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AbilitySystemToGiveTo);

	// Ability sets can only be given by the server.
	if (!AbilitySystemToGiveTo->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Grant each ability.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FCrashAbilitySet_GameplayAbility& AbilityToGive = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGive.GameplayAbility))
		{
			ABILITY_LOG(Error, TEXT("Ability set [%s] failed to grant ability at index [%i]. Ability is not valid."), *GetNameSafe(this), AbilityIndex);
			continue;
		}

		// Create a spec with which to give the ability.
		UCrashGameplayAbilityBase* AbilityCDO = AbilityToGive.GameplayAbility->GetDefaultObject<UCrashGameplayAbilityBase>();
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityCDO, AbilityToGive.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		// Grant the ability to the specified ASC.
		const FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemToGiveTo->GiveAbility(AbilitySpec);

		// Cache a handle to the given ability.
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Apply each gameplay effect.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FCrashAbilitySet_GameplayEffect& EffectToApply = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToApply.GameplayEffect))
		{
			ABILITY_LOG(Error, TEXT("Ability set [%s] failed to apply an effect at index [%i]. Effect is not valid."), *GetNameSafe(this), EffectIndex);
			continue;
		}

		// Apply the gameplay effect to the specified ASC.
		const UGameplayEffect* GameplayEffect = EffectToApply.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = AbilitySystemToGiveTo->ApplyGameplayEffectToSelf(GameplayEffect, EffectToApply.EffectLevel, AbilitySystemToGiveTo->MakeEffectContext());

		// Cache a handle to this gameplay effect while it's active.
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	// Add each attribute set.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributeSets.Num(); ++SetIndex)
	{
		const FCrashAbilitySet_AttributeSet& SetToGrant = GrantedAttributeSets[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			ABILITY_LOG(Error, TEXT("Ability set [%s] failed to add an attribute set at index [%i]. Attribute set is not valid."), *GetNameSafe(this), SetIndex);
			continue;
		}

		// Add the attribute set to the specified ASC.
		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemToGiveTo->GetOwner(), SetToGrant.AttributeSet);
		AbilitySystemToGiveTo->AddAttributeSetSubobject(NewSet);

		// Cache a pointer to the new attribute set.
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}
