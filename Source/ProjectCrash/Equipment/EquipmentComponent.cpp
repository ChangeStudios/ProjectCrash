// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentComponent.h"

#include "EquipmentDefinition.h"
#include "EquipmentInstance.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UEquipmentComponent::UninitializeComponent()
{
	TArray<UEquipmentInstance*> EquipmentInstances;

	// Copy the list of equipment instances, so we don't modify the equipment list.
	for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		EquipmentInstances.Add(Entry.EquipmentInstance);
	}

	// Unequip all equipment when this component is uninitialized.
	for (UEquipmentInstance* EquipmentInstance : EquipmentInstances)
	{
		UnequipItem(EquipmentInstance);
	}

	Super::UninitializeComponent();
}

bool UEquipmentComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Register each equipment instance equipped by this component as a replicated sub-object.
	for (FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UEquipmentInstance* EquipmentInstance = Entry.EquipmentInstance;

		if (IsValid(EquipmentInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(EquipmentInstance, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UEquipmentComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	/* Register existing equipment instances as replicated sub-objects. This is needed to register equipment instances
	 * as replicated sub-objects if they were equipped before this equipment component began replicating. */
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
		{
			UEquipmentInstance* EquipmentInstance = Entry.EquipmentInstance;

			if (IsValid(EquipmentInstance))
			{
				AddReplicatedSubObject(EquipmentInstance);
			}
		}
	}
}

UEquipmentInstance* UEquipmentComponent::EquipItem(UEquipmentDefinition* EquipmentDefinition)
{
	UEquipmentInstance* NewEquipment = nullptr;

	if (EquipmentDefinition != nullptr)
	{
		// Create and equip a new instance for the given equipment through the equipment list.
		NewEquipment = EquipmentList.AddEntry(EquipmentDefinition);

		if (NewEquipment != nullptr)
		{
			/* Notify the new equipment that it's been equipped on the server. The equipment list will handle this for
			 * clients. */
			NewEquipment->OnEquipped();

			// Replicate the new equipment instance as a sub-object.
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(NewEquipment);
			}
		}
	}
	else
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to equip equipment without a valid definition on pawn [%s]."), *GetNameSafe(GetPawn<APawn>()));
	}

	return NewEquipment;
}

void UEquipmentComponent::UnequipItem(UEquipmentInstance* EquipmentInstance)
{
	if (EquipmentInstance != nullptr)
	{
		// Stop replicate the equipment instance as a sub-object.
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(EquipmentInstance);
		}

		/* Notify the equipment that it's being unequipped on the server. The equipment list will handle this for
		 * clients. */
		EquipmentInstance->OnUnequipped();

		// Unequip and destroy the equipment instance.
		EquipmentList.RemoveEntry(EquipmentInstance);
	}
	else
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to unequip null equipment instance on pawn [%s]."), *GetNameSafe(GetPawn<APawn>()));
	}
}

TArray<UEquipmentInstance*> UEquipmentComponent::GetAllEquipment() const
{
	return EquipmentList.GetAllEquipment();
}

TArray<UEquipmentInstance*> UEquipmentComponent::GetAllEquipmentOfDefinition(UEquipmentDefinition* EquipmentToFind) const
{
	TArray<UEquipmentInstance*> EquipmentInstances;

	// Collect valid equipment instances with the given definition.
	for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UEquipmentInstance* EquipmentInstance = Entry.EquipmentInstance;

		if (IsValid(EquipmentInstance))
		{
			if (EquipmentInstance->GetEquipmentDefinition() == EquipmentToFind)
			{
				EquipmentInstances.Add(EquipmentInstance);
			}
		}
	}

	return EquipmentInstances;
}

TArray<UEquipmentInstance*> UEquipmentComponent::GetAllEquipmentOfType(TSubclassOf<UEquipmentInstance> EquipmentInstanceType) const
{
	TArray<UEquipmentInstance*> EquipmentInstances;

	// Collect valid equipment instances of the given type.
	for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		if (IsValid(Entry.EquipmentInstance) && (Entry.EquipmentInstance->IsA(EquipmentInstanceType)))
		{
			EquipmentInstances.Add(Entry.EquipmentInstance);
		}
	}

	return EquipmentInstances;
}

UEquipmentInstance* UEquipmentComponent::GetFirstEquipmentByDefinition(UEquipmentDefinition* EquipmentToFind) const
{
	for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UEquipmentInstance* EquipmentInstance = Entry.EquipmentInstance;

		if (IsValid(EquipmentInstance))
		{
			if (EquipmentInstance->GetEquipmentDefinition() == EquipmentToFind)
			{
				return EquipmentInstance;
			}
		}
	}

	// An instance of the specified equipment is not equipped.
	return nullptr;
}

UEquipmentInstance* UEquipmentComponent::GetFirstEquipmentByType(TSubclassOf<UEquipmentInstance> EquipmentInstanceType) const
{
	for (const FEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		if (IsValid(Entry.EquipmentInstance) && (Entry.EquipmentInstance->IsA(EquipmentInstanceType)))
		{
			return Entry.EquipmentInstance;
		}
	}

	// An instance of the specified type is not equipped.
	return nullptr;
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipmentComponent, EquipmentList);
}