// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"

UEquipmentComponent::UEquipmentComponent()
{
	// Equipment is server-authoritative and replicated.
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that this component is owned by a character class. It cannot be used by any other classes.
	const ACharacter* Char = GetOwner<ACharacter>();
	ensureAlwaysMsgf((Char != nullptr), TEXT("EquipmentComponent was registered on [%s]. This component can only be used on classes of the ACharacter type."), *GetNameSafe(GetOwner()));

	// Ensure that this component's owner only has one EquipmentComponent.
	TArray<UActorComponent*> EquipmentComponents;
	Char->GetComponents(UEquipmentComponent::StaticClass(), EquipmentComponents);
	ensureAlwaysMsgf((EquipmentComponents.Num() == 1), TEXT("%i instances of EquipmentComponent were found on %s. Only one EquipmentComponent may exist on any actor."), EquipmentComponents.Num(), *GetNameSafe(GetOwner()));
}

void UEquipmentComponent::SendEquipmentEffectEvent(FGameplayTag EffectEvent)
{
}

void UEquipmentComponent::EquipSet(UEquipmentSet* SetToEquip)
{
	check(GetOwner()->HasAuthority());
	check(SetToEquip);

	/* Update the currently equipped set. The previously equipped set will be unequipped by the OnRep, and the new set
	 * will be equipped. */
	UEquipmentSet* PreviouslyEquippedSet = EquippedSet;
	EquippedSet = SetToEquip;

	// Manually call the OnRep on the server.
	OnRep_EquippedSet(PreviouslyEquippedSet);
}

void UEquipmentComponent::TemporarilyEquipSet(UEquipmentSet* SetToTemporarilyEquip)
{
	check(GetOwner()->HasAuthority());
	check(SetToTemporarilyEquip);

	/* Update the currently equipped set. If there was a temporarily equipped set, it will be unequipped by the OnRep;
	 * otherwise, EquippedSet will be unequipped instead, before the new temporary set is equipped. */
	UEquipmentSet* PreviousTemporarilyEquippedSet = TemporarilyEquippedSet;
	TemporarilyEquippedSet = SetToTemporarilyEquip;

	// Manually call the OnRep on the server.
	OnRep_TemporarilyEquippedSet(PreviousTemporarilyEquippedSet);
}

bool UEquipmentComponent::UnequipTemporarySet()
{
	check(GetOwner()->HasAuthority());

	// Unequip the current temporarily equipped set, if there is one.
	if (UEquipmentSet* PreviousTemporarilyEquippedSet = TemporarilyEquippedSet)
	{
		TemporarilyEquippedSet = nullptr;
		OnRep_TemporarilyEquippedSet(PreviousTemporarilyEquippedSet);
		return true;
	}

	// If there isn't a temporarily equipped set, do nothing.
	return false;
}

void UEquipmentComponent::EquipSet_Internal(UEquipmentSet* SetToEquip)
{
}

void UEquipmentComponent::UnequipSet_Internal(bool bUnequipTemporarySet, bool bUnequipTemporarily)
{
}

void UEquipmentComponent::OnRep_EquippedSet(UEquipmentSet* PreviouslyEquippedSet)
{
	// Unequip the previously equipped set, if there was one.
	if (PreviouslyEquippedSet)
	{
		UnequipSet_Internal(false, false);
	}

	// Equip the new set.
	if (EquippedSet)
	{
		EquipSet_Internal(EquippedSet);
	}
}

void UEquipmentComponent::OnRep_TemporarilyEquippedSet(UEquipmentSet* PreviouslyTemporarilyEquippedSet)
{
	// If a temporary set was previously equipped, unequip it.
	if (PreviouslyTemporarilyEquippedSet)
	{
		UnequipSet_Internal(true, false);
	}
	/* If a persistent equipment set was previously equipped, and a new temporary set is overriding it, temporarily
	 * unequip the persistent set. */
	else if (TemporarilyEquippedSet)
	{
		UnequipSet_Internal(false, true);
	}

	// If a new set was just temporarily equipped, equip it.
	if (TemporarilyEquippedSet)
	{
		EquipSet_Internal(TemporarilyEquippedSet);
	}
	// If the temporarily equipped set was just unequipped, re-equip EquippedSet.
	else
	{
		EquipSet_Internal(EquippedSet);
	}
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, EquippedSet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, TemporarilyEquippedSet, COND_None, REPNOTIFY_Always);
}
