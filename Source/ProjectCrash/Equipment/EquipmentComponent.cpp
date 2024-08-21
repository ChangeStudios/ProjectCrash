// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentComponent.h"

#include "EquipmentDefinition.h"
#include "EquipmentInstance.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CrashLogging.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryItemInstance.h"
#include "Inventory/Traits/ItemTrait_Equippable.h"
#include "Net/UnrealNetwork.h"

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	CurrentEquipment(nullptr)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Try to auto-equip an item on initialization.
	if (HasAuthority())
	{
		APawn* Pawn = GetPawnChecked<APawn>();

		if (Pawn->GetController())
		{
			AutoEquipFirstItem();
		}
		else
		{
			Pawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UEquipmentComponent::OnControllerChanged);
		}
	}
}

void UEquipmentComponent::UninitializeComponent()
{
	// Unequip the current item when this component is uninitialized.
	if (HasAuthority() && CurrentEquipment)
	{
		UnequipItem();
	}

	Super::UninitializeComponent();
}

bool UEquipmentComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Replicate the current equipment instance as a sub-object of this component.
	if (IsValid(CurrentEquipment))
	{
		bWroteSomething |= Channel->ReplicateSubobject(CurrentEquipment, *Bunch, *RepFlags);
	}

	return bWroteSomething;
}

void UEquipmentComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	/* Register the existing equipment instance as a replicated sub-object. This is needed to register the current
	 * equipment if it was equipped before this equipment component began replicating. */
	if (IsUsingRegisteredSubObjectList())
	{
		if (IsValid(CurrentEquipment))
		{
			AddReplicatedSubObject(CurrentEquipment);
		}
	}
}

UEquipmentInstance* UEquipmentComponent::EquipItem(UEquipmentDefinition* EquipmentDefinition, UObject* Instigator)
{
	UEquipmentInstance* NewEquipment = nullptr;

	check(HasAuthority());

	if (EquipmentDefinition != nullptr)
	{
		if (CurrentEquipment)
		{
			UnequipItem(false);
		}

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
		if (!ensureAlwaysMsgf((EquipmentSkin != nullptr), TEXT("Tried to equip equipment [%s], but it does not have a default equipment skin. Equipment cannot be spawned without a skin (the skin itself can be left empty if desired)."), *GetNameSafe(EquipmentDefinition)))
		{
			return nullptr;
		}

		// Create a new instance for the given equipment.
		NewEquipment = NewObject<UEquipmentInstance>(GetOwner(), EquipmentDefinition->EquipmentInstanceClass);

		// Initialize the equipment. This grants the equipment's abilities and spawns its equipment actors.
		NewEquipment->InitializeEquipment(EquipmentDefinition, EquipmentSkin, Instigator);

		// Update our current equipment. This calls OnEquipped on clients.
		CurrentEquipment = NewEquipment;
		MARK_PROPERTY_DIRTY_FROM_NAME(UEquipmentComponent, CurrentEquipment, this);
		GetOwner()->ForceNetUpdate();

		// Notify the new equipment that it's been equipped on the server.
		NewEquipment->OnEquipped();

		// Replicate the new equipment instance as a sub-object.
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(NewEquipment);
		}
	}
	else
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to equip equipment without a valid definition on pawn [%s]."), *GetNameSafe(GetPawn<APawn>()));
	}

	return NewEquipment;
}

void UEquipmentComponent::UnequipItem(bool bTryAutoEquip)
{
	if (CurrentEquipment != nullptr)
	{
		// Cache the current equipment's associated item, so it doesn't accidentally get re-equipped from auto-equip.
		UInventoryItemInstance* LastEquippedItem = Cast<UInventoryItemInstance>(CurrentEquipment->GetInstigator());

		// Stop replicate the equipment instance as a sub-object.
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(CurrentEquipment);
		}

		// Notify the equipment that it's being unequipped on the server.
		CurrentEquipment->OnUnequipped();

		// Uninitialize and destroy the equipment instance.
		CurrentEquipment->UninitializeEquipment();

		// Clear CurrentEquipment to trigger OnUnequipped on clients.
		CurrentEquipment = nullptr;
		MARK_PROPERTY_DIRTY_FROM_NAME(UEquipmentComponent, CurrentEquipment, this);
		GetOwner()->ForceNetUpdate();

		// Try to auto-equip a new item, if desired.
		if (bTryAutoEquip)
		{
			AutoEquipFirstItem({ LastEquippedItem });
		}
	}
	else
	{
		EQUIPMENT_LOG(Error, TEXT("Attempted to unequip null equipment instance on pawn [%s]."), *GetNameSafe(GetPawn<APawn>()));
	}
}

void UEquipmentComponent::OnRep_CurrentEquipment(UEquipmentInstance* PreviousEquipment)
{
	if (PreviousEquipment)
	{
		PreviousEquipment->OnUnequipped();
	}

	if (CurrentEquipment)
	{
		CurrentEquipment->OnEquipped();
	}
}

void UEquipmentComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	AutoEquipFirstItem();
}

template <typename T>
T* UEquipmentComponent::GetEquipment()
{
	if (CurrentEquipment && CurrentEquipment->IsA(T::StaticClass()))
	{
		return (T*)CurrentEquipment;
	}

	return nullptr;
}

UEquipmentComponent* UEquipmentComponent::FindEquipmentComponentFromItem(UInventoryItemInstance* ItemInstance)
{
	UEquipmentComponent* EquipmentComp = nullptr;

	AActor* Owner = ItemInstance->GetOwner();
	if (ensure(Owner))
	{
		/* Item owner should be a controller when owned by a player (as opposed to being owned by a pick-up actor,
		 * which will not have an equipment component.) */
		if (AController* Controller = Cast<AController>(Owner))
		{
			if (Controller->GetPawn())
			{
				EquipmentComp = FindEquipmentComponent(Controller->GetPawn());
			}
		}
	}

	return EquipmentComp;
}

void UEquipmentComponent::AutoEquipFirstItem(TArray<UInventoryItemInstance*> ItemsToIgnore)
{
	if (CurrentEquipment)
	{
		return;
	}

	// Try to find an inventory component on the owning controller.
	if (UInventoryComponent* InventoryComp = UInventoryComponent::FindInventoryComponent(GetController<AController>()))
	{
		// Search for any equippable items that are set to auto-equip, and try to equip one.
		for (UInventoryItemInstance* Item : InventoryComp->GetAllItems())
		{
			if (ItemsToIgnore.Contains(Item))
			{
				continue;
			}

			if (const UItemTrait_Equippable* EquippableTrait = Item->FindTraitByClass<UItemTrait_Equippable>())
			{
				if (EquippableTrait->bAutoEquip)
				{
					if (EquipItem(EquippableTrait->EquipmentDefinition, Item))
					{
						// Stop after an item is successfully equipped.
						break;
					}
				}
			}
		}
	}
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CurrentEquipment, SharedParams);
}
