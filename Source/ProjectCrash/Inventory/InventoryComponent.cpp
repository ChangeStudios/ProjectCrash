// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryComponent.h"

#include "CrashGameplayTags.h"
#include "InventoryItemDefinition.h"
#include "InventoryItemInstance.h"
#include "Characters/PawnExtensionComponent.h"
#include "Characters/Data/PawnData.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"
#include "Player/CrashPlayerState.h"

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	InventoryList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (HasAuthority())
	{
		// Start listening for pawn possession to add new pawns' default items.
		AController* Controller = GetControllerChecked<AController>();
		Controller->OnPossessedPawnChanged.AddDynamic(this, &UInventoryComponent::OnPawnChanged);
	}
}

void UInventoryComponent::UninitializeComponent()
{
	if (HasAuthority())
	{
		// Stop listening for pawn changes.
		if (AController* Controller = GetController<AController>())
		{
			Controller->OnPossessedPawnChanged.RemoveAll(this);
		}

		// Stop listening for pawn data changes.
		if (ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>())
		{
			CrashPS->PawnDataChangedDelegate.RemoveAll(this);
		}

		// Clear AddPawnItems callback.
		ReadyForItems.Clear();
	}

	Super::UninitializeComponent();
}

void UInventoryComponent::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	check(HasAuthority());

	// When a new pawn is possessed, grant its pawn data's default items after it's been initialized.
	if (IsValid(NewPawn) && (NewPawn != OldPawn))
	{
		// Pawn data is only used by pawns with an extension component.
		if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
		{
			/* Register AddPawnItems to when the pawn extension component reaches GameplayReady. This it to guarantee
			 * that its pawn data has been set. */
			ReadyForItems.BindDynamic(this, &UInventoryComponent::AddPawnItems);
			PawnExtComp->RegisterAndCallForInitStateChange(CrashGameplayTags::TAG_InitState_GameplayReady, MoveTemp(ReadyForItems), true);

			// TODO: Listen for death from ASC
		}
	}

	/* When the inventory's owning player state switches pawns (changes character), treat it as a death. We bind this
	 * here because we can't guarantee the player state will be valid in InitializeComponent. */
	if (ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>())
	{
		CrashPS->PawnDataChangedDelegate.AddUniqueDynamic(this, &UInventoryComponent::OnPawnDataChanged);
	}
}

void UInventoryComponent::AddPawnItems(const FActorInitStateChangedParams& Params)
{
	// Add each default item defined in the pawn data of this inventory's owner's pawn.
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (const UPawnData* PawnData = PawnExtComp->GetPawnData<UPawnData>())
			{
				for (TSubclassOf<UInventoryItemDefinition> Item : PawnData->DefaultItems)
				{
					PawnItems.Add(AddItemByDefinition(Item));
				}
			}
		}
	}

	// Clear callback.
	ReadyForItems.Clear();
}

void UInventoryComponent::OnPawnDeath()
{
	// Trigger death logic for each item granted by the dying pawn.
	for (UInventoryItemInstance* Item : PawnItems)
	{
		if (const TSubclassOf<UInventoryItemDefinition> ItemDef = Item->GetItemDefinition())
		{
			switch (GetDefault<UInventoryItemDefinition>(ItemDef)->DeathBehavior)
			{
				// Remove items that should be destroyed when the pawn dies.
			case EItemDeathBehavior::Destroyed:
				{
					RemoveItem(Item);
					break;
				}
				// Drop items that should be dropped, if possible. Otherwise, remove them.
			case EItemDeathBehavior::Dropped:
				{
					if (GetDefault<UInventoryItemDefinition>(ItemDef)->FindTraitByClass(nullptr)) // TODO: implement drop logic.
					{
						// Drop item.
					}
					else
					{
						RemoveItem(Item);
					}

					break;
				}
				// Do nothing for persistent items.
			case EItemDeathBehavior::Persistent:
			default:
				break;
			}
		}
	}
}

void UInventoryComponent::OnPawnDataChanged(const UPawnData* OldPawnData, const UPawnData* NewPawnData)
{
	// Treat changing pawn data (i.e. switching characters) as a death.
	OnPawnDeath();
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Register each item instance in this inventory as a replicated sub-object.
	for (FInventoryListEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}

	return bWroteSomething;
}

void UInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	/* Register existing item instances as replicated sub-objects. This is needed to register item instances as
	 * replicated sub-objects if they were added to the inventory before this inventory component began replicating. */
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryListEntry& Entry : InventoryList.Entries)
		{
			UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

bool UInventoryComponent::CanAddItem(TSubclassOf<UInventoryItemDefinition> ItemDefinition)
{
	// TODO: Check for item restrictions (uniqueness, count, etc.).

	return true;
}

UInventoryItemInstance* UInventoryComponent::AddItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDefinition)
{
	UInventoryItemInstance* NewItem = nullptr;

	if (ItemDefinition != nullptr)
	{
		// Create a new instance for the given item through the inventory list.
		NewItem = InventoryList.AddEntry(ItemDefinition);

		// Replicate the new item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && NewItem)
		{
			AddReplicatedSubObject(NewItem);
		}
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to add an item without a valid definition to inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}

	return NewItem;
}

void UInventoryComponent::AddItemByInstance(UInventoryItemInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		// Add the item instance to the inventory.
		InventoryList.AddEntry(ItemInstance);

		// Start replicating the item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to add an invalid item to inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}
}

void UInventoryComponent::RemoveItem(UInventoryItemInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		// Stop replicating the item instance as a sub-object.
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}

		// Remove the item instance from the inventory.
		InventoryList.RemoveEntry(ItemInstance);
	}
	else
	{
		INVENTORY_LOG(Error, TEXT("Attempted to remove a null item from inventory owned by [%s]."), *GetNameSafe(GetOwner()));
	}
}

void UInventoryComponent::ClearInventory()
{
	// Remove each item in this inventory.
	for (auto EntryIt = InventoryList.Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryListEntry& Entry = *EntryIt;

		if (Entry.ItemInstance != nullptr)
		{
			RemoveItem(Entry.ItemInstance);
		}
	}
}

TArray<UInventoryItemInstance*> UInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UInventoryItemInstance* UInventoryComponent::FindFirstItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemToFind) const
{
	for (const FInventoryListEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* ItemInstance = Entry.ItemInstance;

		if (IsValid(ItemInstance))
		{
			if (ItemInstance->GetItemDefinition() == ItemToFind)
			{
				return ItemInstance;
			}
		}
	}

	// An instance of the specified item was not found in this inventory.
	return nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryList);
}
