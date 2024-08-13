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
    return FString::Printf(TEXT("%s (%s)"), *GetNameSafe(EquipmentInstance), *GetNameSafe(EquipmentInstance ? EquipmentInstance->GetEquipmentDefinition() : nullptr));
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
	NewEntry.EquipmentInstance = NewObject<UEquipmentInstance>(OwningComponent->GetOwner(), EquipmentDefinition->EquipmentInstanceClass); // Outer object is the owning actor because equipment instances only exist while equipped.
	NewEquipment = NewEntry.EquipmentInstance;

	// Determine the skin to use for this equipment.
	UEquipmentSkin* EquipmentSkin = nullptr;
		
	// Retrieve equipping player's skin from the skin subsystem if the equipment supports skins.
	if (EquipmentDefinition->EquipmentSkinID.IsValid())
	{
		// TODO: Retrieve the equipping player's skin from the skin subsystem.
	}

	/* If the player does not have a skin for this equipment or this equipment does not support skins, use the
	 * equipment's default skin. */
	if (EquipmentSkin == nullptr)
	{
		EquipmentSkin = EquipmentDefinition->DefaultEquipmentSkin;
	}

	// Equipment cannot be spawned without a skin.
	if (ensureAlwaysMsgf((EquipmentSkin != nullptr), TEXT("Tried to equip equipment [%s], but it does not have a default equipment skin. Equipment cannot be spawned without a skin (the skin itself can be left empty if desired)."), *GetNameSafe(EquipmentDefinition)))
	{
		return nullptr;
	}

	// Initialize the equipment. This grants the equipment's abilities and spawns its equipment actors.
	NewEquipment->InitializeEquipment(EquipmentDefinition, EquipmentSkin);

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
				Entry.EquipmentInstance->GrantedAbilitySetHandles.RemoveFromAbilitySystem(CrashASC);
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