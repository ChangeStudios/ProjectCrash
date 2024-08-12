// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentList.h"

#include "EquipmentActor.h"
#include "EquipmentDefinition.h"
#include "EquipmentInstance.h"
#include "EquipmentSkin.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/CrashLogging.h"

/**
 * FEquipmentListEntry
 */
FString FEquipmentListEntry::GetDebugString() const
{
    return FString::Printf(TEXT("%s (%s)"), *GetNameSafe(EquipmentInstance), *GetNameSafe(EquipmentDefinition));
}



/**
 * FEquipmentList
 */
UEquipmentInstance* FEquipmentList::AddEntry(UEquipmentDefinition* EquipmentDefinition)
{
	UEquipmentInstance* NewEquipment = nullptr;

	check(EquipmentDefinition != nullptr);
	check(OwningComponent);

	AActor* OwningActor = OwningComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// Add a new entry for the new equipment.
	FEquipmentListEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.EquipmentInstance = NewObject<UEquipmentInstance>(OwningComponent->GetOwner(), EquipmentDefinition->EquipmentInstanceClass); // Outer object is the owning actor because equipment instances only exist while equipped.
	NewEquipment = NewEntry.EquipmentInstance;

	// Grant the ability sets that this equipment grants when equipped.
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(OwningActor))
	{
		for (const TObjectPtr<const UCrashAbilitySet>& AbilitySet : EquipmentDefinition->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(CrashASC, &NewEntry.GrantedAbilitySetHandles, NewEquipment);
		}
	}
	else if (EquipmentDefinition->AbilitySetsToGrant.Num()) // Only warn if this equipment is supposed to grant ability sets.
	{
		UE_LOG(LogEquipment, Warning, TEXT("Equipped equipment [%s] on actor [%s] but failed to grant the equipment's ability sets: could not find an ASC for the equipping actor."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(OwningActor));
	}

	// Determine which equipment skin will be used to spawn the new equipment's actors.
	UEquipmentSkin* EquipmentSkinToSpawn = nullptr;

	// TODO: Retrieve equipping player's equipment skin.

	// If the player does not have a skin for this equipment, use the equipment's default skin.
	if (EquipmentSkinToSpawn == nullptr)
	{
		EquipmentSkinToSpawn = EquipmentDefinition->DefaultEquipmentSkin;
	}

	checkf(EquipmentSkinToSpawn, TEXT("Tried to equip equipment [%s], but it does not have a default equipment skin."), *GetNameSafe(EquipmentDefinition));

	// Spawn the new equipment's first- and third-person equipment actors.
	NewEquipment->SpawnEquipmentActors(EquipmentSkinToSpawn->FirstPersonActorsToSpawn, EEquipmentPerspective::FirstPerson);
	NewEquipment->SpawnEquipmentActors(EquipmentSkinToSpawn->ThirdPersonActorsToSpawn, EEquipmentPerspective::ThirdPerson);

	// Mark the new entry for replication.
	MarkItemDirty(NewEntry);

	return NewEquipment;
}

void FEquipmentList::RemoveEntry(UEquipmentInstance* EquipmentInstance)
{
	check(EquipmentInstance != nullptr);
	check(OwningComponent);

	AActor* OwningActor = OwningComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// Find the equipment to remove.
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FEquipmentListEntry& Entry = *EntryIt;
		if (Entry.EquipmentInstance == EquipmentInstance)
		{
			// Remove any granted ability sets when this equipment is unequipped.
			if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(OwningActor))
			{
				Entry.GrantedAbilitySetHandles.RemoveFromAbilitySystem(CrashASC);
			}

			// Destroy the equipment's spawned actors.
			EquipmentInstance->DestroyEquipmentActors();

			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

TArray<UEquipmentInstance*> FEquipmentList::GetAllEquipment() const
{
	// Alloc space for each equipped equipment.
	TArray<UEquipmentInstance*> Equipment;
	Equipment.Reserve(Entries.Num());

	// Collect the equipment instances from each equipment entry.
	for (const FEquipmentListEntry& Entry : Entries)
	{
		if (Entry.EquipmentInstance != nullptr)
		{
			Equipment.Add(Entry.EquipmentInstance);
		}
	}

	return Equipment;
}

void FEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// Notify the removed equipment that it was unequipped on the client.
	for (int32 Index : RemovedIndices)
	{
		const FEquipmentListEntry& Entry = Entries[Index];
		if (Entry.EquipmentInstance != nullptr)
		{
			Entry.EquipmentInstance->OnUnequipped();
		}
	}
}

void FEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// Notify the added equipment that it was equipped on the client.
	for (int32 Index : AddedIndices)
	{
		const FEquipmentListEntry& Entry = Entries[Index];
		if (Entry.EquipmentInstance != nullptr)
		{
			Entry.EquipmentInstance->OnEquipped();
		}
	}
}