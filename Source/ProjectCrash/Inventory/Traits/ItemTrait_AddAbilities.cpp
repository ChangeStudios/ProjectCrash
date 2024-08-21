// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/Traits/ItemTrait_AddAbilities.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Inventory/InventoryItemInstance.h"

void UItemTrait_AddAbilities::OnItemEnteredInventory(UInventoryItemInstance* ItemInstance) const
{
	check(ItemInstance);

	AActor* Owner = ItemInstance->GetOwner();
	check(Owner->HasAuthority());

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		// Allocate space for the abilities, effects, and attribute sets we'll grant.
		UInventoryItemInstance::FItemAbilityHandles& AbilityHandles = ItemInstance->GrantedItemAbilities;
		AbilityHandles.Abilities.Reserve(Abilities.Num());
		AbilityHandles.Effects.Reserve(Effects.Num());
		AbilityHandles.AttributeSets.Reserve(AttributeSets.Num());

		// Grant abilities.
		for (const TSubclassOf<UGameplayAbility>& Ability : Abilities)
		{
			if (Ability != nullptr)
			{
				// Grant the ability.
				FGameplayAbilitySpec NewAbilitySpec(Ability);
				FGameplayAbilitySpecHandle GrantedAbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

				// Cache the granted ability handle.
				AbilityHandles.Abilities.Add(GrantedAbilityHandle);
			}
		}

		// Apply effects.
		for (const TSubclassOf<UGameplayEffect>& Effect : Effects)
		{
			if (Effect != nullptr)
			{
				// Apply the effect.
				FGameplayEffectSpecHandle NewEffectSpec = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1.0f, AbilitySystemComponent->MakeEffectContext());
				const FActiveGameplayEffectHandle& AppliedEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewEffectSpec.Data);

				// Cache the applied effect handle.
				AbilityHandles.Effects.Add(AppliedEffectHandle);
			}
		}

		// Add attribute sets.
		for (const FAbilitiesItemTraitEntry_AttributeSet& AttributeSet : AttributeSets)
		{
			if (AttributeSet.AttributeSetClass != nullptr)
			{
				// Create the new attribute set.
				UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), AttributeSet.AttributeSetClass);

				// Initialize the attribute set with the initialization data table, if one was given.
				if (AttributeSet.InitializationData != nullptr)
				{
					NewSet->InitFromMetaDataTable(AttributeSet.InitializationData);
				}

				// Cache a reference to the new attribute set.
				AbilityHandles.AttributeSets.Add(NewSet);
				AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
			}
		}
	}
}

void UItemTrait_AddAbilities::OnItemLeftInventory(UInventoryItemInstance* ItemInstance) const
{
	check(ItemInstance);

	AActor* Owner = ItemInstance->GetOwner();
	check(Owner->HasAuthority());

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		/* Because each item can only have one instance of any trait, every ability, effect, and attribute granted by
		 * this item will be from this trait. This means that we can remove every ability, effect, and attribute from
		 * this item without worrying about which was granted by which trait: they were all granted by this trait. */
		UInventoryItemInstance::FItemAbilityHandles& AbilityHandles = ItemInstance->GrantedItemAbilities;

		// Remove abilities.
		for (FGameplayAbilitySpecHandle& Ability : AbilityHandles.Abilities)
		{
			if (Ability.IsValid())
			{
				AbilitySystemComponent->SetRemoveAbilityOnEnd(Ability);
			}
		}

		// Remove effects.
		for (FActiveGameplayEffectHandle& Effect : AbilityHandles.Effects)
		{
			// Items can grant duration-based effects, which may have removed themselves.
			if (Effect.IsValid())
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(Effect, 1); // Only remove the stack applied by this trait.
			}
		}

		// Remove attribute sets.
		for (UAttributeSet* AttributeSet : AbilityHandles.AttributeSets)
		{
			if (AttributeSet != nullptr)
			{
				AbilitySystemComponent->RemoveSpawnedAttribute(AttributeSet);
			}
		}

		// Clear the item's ability handles.
		AbilityHandles.Abilities.Reset();
		AbilityHandles.Effects.Reset();
		AbilityHandles.AttributeSets.Reset();
	}
}