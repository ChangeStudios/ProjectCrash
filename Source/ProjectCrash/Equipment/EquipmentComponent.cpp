// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "EquipmentLog.h"
#include "EquipmentSet.h"
#include "GameFramework/Character.h"

UEquipmentComponent::UEquipmentComponent()
{
}

void UEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that this component is in a character class.
	const ACharacter* Char = GetOwner<ACharacter>();
	ensureAlwaysMsgf((Char != nullptr), TEXT("EquipmentComponent on [%s] can only be added to character actors."), *GetNameSafe(GetOwner()));

	// Ensure that this is the only instance of this component in its owner.
	TArray<UActorComponent*> ASCExtensionComponents;
	Char->GetComponents(UEquipmentComponent::StaticClass(), ASCExtensionComponents);
	ensureAlwaysMsgf((ASCExtensionComponents.Num() == 1), TEXT("Only one EquipmentComponent should exist on [%s]."), *GetNameSafe(GetOwner()));
}

void UEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();

	/* Bind the OnTemporarilyUnequippedChanged callback to when this component's owner's ASC gains or loses the
	 * TemporarilyUnequipped tag. */
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		TemporarilyUnequippedDelegate = ASC->RegisterGameplayTagEvent(CrashGameplayTags::TAG_State_TemporarilyUnequipped, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UEquipmentComponent::OnTemporarilyUnequippedChanged);
	}
}

UEquipmentSet* UEquipmentComponent::EquipEquipmentSet(UEquipmentSet* SetToEquip)
{
	return nullptr;
}

bool UEquipmentComponent::EquipSet_Internal(UEquipmentSet* SetToEquip)
{
	check(SetToEquip);

	/* We cannot equip a new equipment set if we already have on equipped. EquipEquipmentSet handles this for us, which
	 * is why we don't call this function directly. */
	if (CurrentEquipmentSet)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to equip equipment set [%s] on actor [%s], but the actor's current equipment set, [%s], must be unequipped first."), *SetToEquip->GetName(), *GetNameSafe(GetOwner()), *CurrentEquipmentSet->GetName());
		return false;
	}

	/*
	 * - Spawn actors.
	 * - Attach actors.
	 * - update animations.
	 * - Grant ability set.
	 * - Play equip animation.
	 */

	return true;
}

bool UEquipmentComponent::UnequipSet_Internal()
{
	/* If we don't have an equipment set equipped, we don't need to do anything. */
	if (!CurrentEquipmentSet)
	{
		return false;
	}

	/*
	 * - Destroy actors.
	 * - (Maybe update animations to a default animation set?).
	 * - Remove ability set.
	 */

	return true;
}

void UEquipmentComponent::OnTemporarilyUnequippedChanged(const FGameplayTag Tag, int32 NewCount)
{
	/* Temporarily unequip this character's current equipment set when the TemporarilyUnequipped tag is gained. */
	if (NewCount > 0)
	{
		
	}
	/* Re-equip this character's current equipment set when all TemporarilyUnequipped tags have been removed. */
	else
	{
		
	}
}
