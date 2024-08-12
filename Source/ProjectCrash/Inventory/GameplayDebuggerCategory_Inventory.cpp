// Copyright Samuel Reitich. All rights reserved.


#include "GameplayDebuggerCategory_Inventory.h"

#include "InventoryItemInstance.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "InventoryComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerState.h"

FGameplayDebuggerCategory_Inventory::FGameplayDebuggerCategory_Inventory()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_Inventory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	// Reset inventory contents.
	DataPack.Items.Reset();


	// Try to find an inventory associated with the debug actor.
	UInventoryComponent* InventoryComp = nullptr;

	// Inventory is owned by observed actor (e.g. AI character).
	if (!InventoryComp)
	{
		InventoryComp = DebugActor->FindComponentByClass<UInventoryComponent>();
	}

	// Inventory is owned by player state (e.g. player).
	if (!InventoryComp)
	{
		if (APawn* DebugActorAsPawn = Cast<APawn>(DebugActor))
		{
			if (APlayerState* PS = DebugActorAsPawn->GetPlayerState())
			{
				InventoryComp = PS->FindComponentByClass<UInventoryComponent>();
			}
		}
	}


	// Inventory was found.
	if (InventoryComp)
	{
		DataPack.bHasInventory = 1;
		DataPack.InventoryOwnerName = GetNameSafe(InventoryComp->GetOwner());

		// Convert every item in the observed inventory into debug data before serializing.
		for (const UInventoryItemInstance* Item : InventoryComp->GetAllItems())
		{
			// Include null items for debugging.
			if (Item == nullptr)
			{
				FRepData::FItem& RepItem = DataPack.Items.AddDefaulted_GetRef();
				continue;
			}

			FRepData::FItem& RepItem = DataPack.Items.AddDefaulted_GetRef();

			// Name.
			RepItem.DisplayName = GetNameSafe(Item);

			// Item def.
			RepItem.ItemDef = GetNameSafe(Item->GetItemDefinition());

			// Owner.
			RepItem.ItemOwnerName = GetNameSafe(Item->GetOwner());

			// Stat tags.
			Item->StatTags.GetDebugStrings(RepItem.StatTags);

			// TODO: Abilities
			// TODO: Effects
			// TODO: Attributes
		}
	}
	// Observed actor does not have an inventory component.
	else
	{
		DataPack.bHasInventory = 0;
		DataPack.InventoryOwnerName = GetNameSafe(DebugActor);
	}
}

void FGameplayDebuggerCategory_Inventory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Null inventory case.
	if (!DataPack.bHasInventory)
	{
		CanvasContext.Printf(TEXT("{red}Debug target {yellow}%s{red} does not have an inventory."), *DataPack.InventoryOwnerName);
		return;
	}


	// Inventory Contents (Owner):
	CanvasContext.Printf(TEXT("Inventory Contents ({yellow}%s{white}):"), *DataPack.InventoryOwnerName);

	if (DataPack.Items.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t{red}Inventory is empty."));
		return;
	}


	for (FRepData::FItem& Item : DataPack.Items)
	{
		// Spacer between items.
		CanvasContext.Printf(TEXT(""));


		// Null item case.
		if (Item.DisplayName.IsEmpty())
		{
			CanvasContext.Printf(TEXT("\t{red}[Missing item!]"));
			continue;
		}


		//	Item Name
		CanvasContext.Printf(TEXT("\t{green}%s"), *Item.DisplayName);

		//		Instance of: Item Definition
		CanvasContext.Printf(TEXT("\t\tInstance of: {yellow}%s"), *Item.ItemDef);

		//		Owner: Owning Actor
		CanvasContext.Printf(TEXT("\t\tOwner: {yellow}%s"), *Item.ItemOwnerName);

		//		Current Statistics:
		CanvasContext.Printf(TEXT("\t\tCurrent Statistics:"));

		//			Tag Name (xCount)
		if (Item.StatTags.Num() == 0)
		{
			CanvasContext.Printf(TEXT("\t\t\t{red}No stats."));
		}
		else
		{
			for (FString& StatTag : Item.StatTags)
			{
				CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *StatTag);
			}
		}

		// TODO: Abilities
		// TODO: Effects
		// TODO: Attributes
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Inventory::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Inventory());
}

void FGameplayDebuggerCategory_Inventory::FRepData::Serialize(FArchive& Ar)
{
	// Items.
	int32 NumItems = Items.Num();
	Ar << NumItems;
	if (Ar.IsLoading())
	{
		Items.SetNum(NumItems);
	}

	for (int32 Idx = 0; Idx < NumItems; Idx++)
	{
		Ar << Items[Idx].DisplayName;
		Ar << Items[Idx].ItemDef;
		Ar << Items[Idx].ItemOwnerName;
		Ar << Items[Idx].StatTags;
		Ar << Items[Idx].GrantedAbilities;
		Ar << Items[Idx].AppliedEffects;
		Ar << Items[Idx].AddedAttributeSets;
	}

	// Data.
	Ar << InventoryOwnerName;

	// Bit flags.
	uint32 BitFlags =
		((bHasInventory ? 1 : 0) << 0);

	Ar << BitFlags;

	bHasInventory = (BitFlags & (1 << 0)) != 0;
}

#endif // WITH_GAMEPLAY_DEBUGGER