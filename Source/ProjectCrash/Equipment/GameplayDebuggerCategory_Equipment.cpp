// Copyright Samuel Reitich. All rights reserved.


#include "GameplayDebuggerCategory_Equipment.h"

#include "EquipmentActor.h"
#include "EquipmentDefinition.h"
#include "EquipmentInstance.h"
#include "Inventory/InventoryItemInstance.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "EquipmentComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"

FGameplayDebuggerCategory_Equipment::FGameplayDebuggerCategory_Equipment()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

void FGameplayDebuggerCategory_Equipment::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack.DebugActorName = GetNameSafe(DebugActor);
	
	// Reset equipment.
	DataPack.Equipment.Reset();


	// Try to find an equipment component on the debug actor.
	UEquipmentComponent* EquipmentComp = nullptr;

	if (APawn* DebugActorAsPawn = Cast<APawn>(DebugActor))
	{
		EquipmentComp = DebugActorAsPawn->FindComponentByClass<UEquipmentComponent>();
	}

	// Debug actor has an equipment component.
	if (EquipmentComp)
	{
		DataPack.bHasEquipment = 1;

		// Convert all equipped items into debug data before serializing.
		for (const UEquipmentInstance* EquipmentInstance : EquipmentComp->GetAllEquipment())
		{
			// Include null items for debugging.
			if (EquipmentInstance == nullptr)
			{
				FRepData::FEquipment& RepItem = DataPack.Equipment.AddDefaulted_GetRef();
				continue;
			}

			FRepData::FEquipment& RepItem = DataPack.Equipment.AddDefaulted_GetRef();

			// Name.
			RepItem.DisplayName = GetNameSafe(EquipmentInstance);

			// Equipment def.
			RepItem.EquipmentDef = GetNameSafe(EquipmentInstance->GetEquipmentDefinition());

			// Instance type.
			RepItem.EquipmentInstanceType = GetNameSafe(EquipmentInstance->GetClass());

			// Inventory item should be the equipment's instigator.
			UObject* Instigator = EquipmentInstance->GetInstigator();
			RepItem.AssociatedInventoryItem = (Instigator && Instigator->IsA(UInventoryItemInstance::StaticClass())) ?
												GetNameSafe(Instigator) :
												FString("None");

			// Spawned equipment actors.
			RepItem.SpawnedEquipmentActors.Reset();
			for (const AEquipmentActor* EquipmentActor : EquipmentInstance->GetSpawnedActors())
			{
				RepItem.SpawnedEquipmentActors.Add(GetNameSafe(EquipmentActor));
			}

			// Ability info.
			EquipmentInstance->GrantedAbilitySetHandles.GetAbilityDebugInfo(RepItem.GrantedAbilities);
			EquipmentInstance->GrantedAbilitySetHandles.GetEffectDebugInfo(RepItem.AppliedEffects);
			EquipmentInstance->GrantedAbilitySetHandles.GetAttributeDebugInfo(RepItem.AddedAttributeSets);
		}
	}
	// Observed actor does not have an equipment component.
	else
	{
		DataPack.bHasEquipment = 0;
	}
}

void FGameplayDebuggerCategory_Equipment::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Null equipment component case.
	if (!DataPack.bHasEquipment)
	{
		CanvasContext.Printf(TEXT("{red}Debug target {yellow}%s{red} cannot use equipment."), *DataPack.DebugActorName);
		return;
	}

	// Current Equipment (Owner):
	CanvasContext.Printf(TEXT("Inventory Contents ({yellow}%s{white}):"), *DataPack.DebugActorName);

	if (DataPack.Equipment.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t{red}No equipped items."));
		return;
	}


	for (FRepData::FEquipment& Equipment : DataPack.Equipment)
	{
		// Spacer between items.
		CanvasContext.Printf(TEXT(""));


		// Null equipment case.
		if (Equipment.DisplayName.IsEmpty())
		{
			CanvasContext.Printf(TEXT("\t{red}[Missing equipment!]"));
			continue;
		}


		//	Equipment Name
		CanvasContext.Printf(TEXT("\t{green}%s"), *Equipment.DisplayName);

		//		Instance of: Equipment Definition
		CanvasContext.Printf(TEXT("\t\tInstance of: {yellow}%s"), *Equipment.EquipmentDef);

		//		Instance Class: Instance Type
		CanvasContext.Printf(TEXT("\t\tInstance Class: {yellow}%s"), *Equipment.EquipmentInstanceType);

		//		Associated Inventory Item: Item
		CanvasContext.Printf(TEXT("\t\tAssociated Inventory Item: {yellow}%s"), *Equipment.AssociatedInventoryItem);

		//		Spawned Actors:
		CanvasContext.Printf(TEXT("\t\tSpawned Actors:"));

		//			Equipment Actor
		if (Equipment.SpawnedEquipmentActors.Num() == 0)
		{
			CanvasContext.Printf(TEXT("\t\t\t{red}No spawned equipment actors."));
		}
		else
		{
			for (FString& ActorName : Equipment.SpawnedEquipmentActors)
			{
				CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *ActorName);
			}
		}

		//		Granted Abilities:
		CanvasContext.Printf(TEXT("\t\tGranted Abilities:"));

		//			Ability
		if (Equipment.GrantedAbilities.Num() == 0)
		{
			CanvasContext.Printf(TEXT("\t\t\t{red}No granted abilities."));
		}
		else
		{
			for (FString& AbilityName : Equipment.GrantedAbilities)
			{
				CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *AbilityName);
			}
		}

		//		Applied Effects:
		CanvasContext.Printf(TEXT("\t\tApplied Effects:"));

		//			Effect
		if (Equipment.AppliedEffects.Num() == 0)
		{
			CanvasContext.Printf(TEXT("\t\t\t{red}No applied effects."));
		}
		else
		{
			for (FString& EffectName : Equipment.AppliedEffects)
			{
				CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *EffectName);
			}
		}

		//		Added Attribute Sets:
		CanvasContext.Printf(TEXT("\t\tAdded Attribute Sets:"));

		//			Attribute Set
		if (Equipment.AddedAttributeSets.Num() == 0)
		{
			CanvasContext.Printf(TEXT("\t\t\t{red}No added attribute sets."));
		}
		else
		{
			for (FString& AttributeSetName : Equipment.AddedAttributeSets)
			{
				CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *AttributeSetName);
			}
		}
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Equipment::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Equipment());
}

void FGameplayDebuggerCategory_Equipment::FRepData::Serialize(FArchive& Ar)
{
	// Equipment.
	int32 NumEquipment = Equipment.Num();
	Ar << NumEquipment;
	if (Ar.IsLoading())
	{
		Equipment.SetNum(NumEquipment);
	}

	for (int32 Idx = 0; Idx < NumEquipment; Idx++)
	{
		Ar << Equipment[Idx].DisplayName;
		Ar << Equipment[Idx].EquipmentDef;
		Ar << Equipment[Idx].EquipmentInstanceType;
		Ar << Equipment[Idx].AssociatedInventoryItem;
		Ar << Equipment[Idx].SpawnedEquipmentActors;
		Ar << Equipment[Idx].GrantedAbilities;
		Ar << Equipment[Idx].AppliedEffects;
		Ar << Equipment[Idx].AddedAttributeSets;
	}

	// Data.
	Ar << DebugActorName;

	// Bit flags.
	uint32 BitFlags =
		((bHasEquipment ? 1 : 0) << 0);

	Ar << BitFlags;

	bHasEquipment = (BitFlags & (1 << 0)) != 0;
}

#endif // WITH_GAMEPLAY_DEBUGGER