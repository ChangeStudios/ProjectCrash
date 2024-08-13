// Copyright Samuel Reitich. All rights reserved.


#include "GameplayDebuggerCategory_Equipment.h"

#include "AbilitySystemGlobals.h"
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

	// Try to find an equipment component on the debug actor.
	UEquipmentComponent* EquipmentComp = nullptr;

	if (APawn* DebugActorAsPawn = Cast<APawn>(DebugActor))
	{
		EquipmentComp = DebugActorAsPawn->FindComponentByClass<UEquipmentComponent>();
	}

	if (EquipmentComp)
	{
		DataPack.bHasEquipmentComp = 1;

		// Convert the current equipment into debug data before serializing.
		if (const UEquipmentInstance* EquipmentInstance = EquipmentComp->GetEquipment())
		{
			DataPack.bHasEquipment = 1;

			// Name.
			DataPack.EquipmentDisplayName = GetNameSafe(EquipmentInstance);

			// Equipment def.
			DataPack.EquipmentDef = GetNameSafe(EquipmentInstance->GetEquipmentDefinition());

			// Instance type.
			DataPack.EquipmentInstanceType = GetNameSafe(EquipmentInstance->GetClass());

			// Inventory item should be the equipment's instigator.
			UObject* Instigator = EquipmentInstance->GetInstigator();
			DataPack.AssociatedInventoryItem = (Instigator && Instigator->IsA(UInventoryItemInstance::StaticClass())) ?
												GetNameSafe(Instigator) :
												FString("None");

			// Spawned equipment actors.
			DataPack.SpawnedEquipmentActors.Reset();
			for (const AEquipmentActor* EquipmentActor : EquipmentInstance->GetSpawnedActors())
			{
				DataPack.SpawnedEquipmentActors.Add(GetNameSafe(EquipmentActor));
			}

			// Ability info.
			EquipmentInstance->GrantedAbilitySetHandles.GetAbilityDebugInfo(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(DebugActor), DataPack.GrantedAbilities);
			EquipmentInstance->GrantedAbilitySetHandles.GetEffectDebugInfo(DataPack.AppliedEffects);
			EquipmentInstance->GrantedAbilitySetHandles.GetAttributeDebugInfo(DataPack.AddedAttributeSets);
		}
		// No current equipment.
		else
		{
			DataPack.bHasEquipment = 0;
		}
	}
	// Observed actor does not have an equipment component.
	else
	{
		DataPack.bHasEquipmentComp = 0;
	}
}

void FGameplayDebuggerCategory_Equipment::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// Null equipment component case.
	if (!DataPack.bHasEquipmentComp)
	{
		CanvasContext.Printf(TEXT("{red}Debug target {yellow}%s{red} cannot use equipment."), *DataPack.DebugActorName);
		return;
	}

	// Current Equipment (Owner):
	CanvasContext.Printf(TEXT("Current Equipment ({yellow}%s{white}):"), *DataPack.DebugActorName);

	if (!DataPack.bHasEquipment)
	{
		CanvasContext.Printf(TEXT("\t{red}No equipped item."));
		return;
	}



	//	Equipment Name
	CanvasContext.Printf(TEXT("\t{green}%s"), *DataPack.EquipmentDisplayName);

	//		Instance of: Equipment Definition
	CanvasContext.Printf(TEXT("\t\tInstance of: {yellow}%s"), *DataPack.EquipmentDef);

	//		Instance Class: Instance Type
	CanvasContext.Printf(TEXT("\t\tInstance Class: {yellow}%s"), *DataPack.EquipmentInstanceType);

	//		Associated Inventory Item: Item
	CanvasContext.Printf(TEXT("\t\tAssociated Inventory Item: {yellow}%s"), *DataPack.AssociatedInventoryItem);

	//		Spawned Actors:
	CanvasContext.Printf(TEXT("\t\tSpawned Actors:"));

	//			Equipment Actor
	if (DataPack.SpawnedEquipmentActors.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t\t\t{red}No spawned equipment actors."));
	}
	else
	{
		for (FString& ActorName : DataPack.SpawnedEquipmentActors)
		{
			CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *ActorName);
		}
	}

	//		Granted Abilities:
	CanvasContext.Printf(TEXT("\t\tGranted Abilities:"));

	//			Ability
	if (DataPack.GrantedAbilities.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t\t\t{red}No granted abilities."));
	}
	else
	{
		for (FString& AbilityName : DataPack.GrantedAbilities)
		{
			CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *AbilityName);
		}
	}

	//		Applied Effects:
	CanvasContext.Printf(TEXT("\t\tApplied Effects:"));

	//			Effect
	if (DataPack.AppliedEffects.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t\t\t{red}No applied effects."));
	}
	else
	{
		for (FString& EffectName : DataPack.AppliedEffects)
		{
			CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *EffectName);
		}
	}

	//		Added Attribute Sets:
	CanvasContext.Printf(TEXT("\t\tAdded Attribute Sets:"));

	//			Attribute Set
	if (DataPack.AddedAttributeSets.Num() == 0)
	{
		CanvasContext.Printf(TEXT("\t\t\t{red}No added attribute sets."));
	}
	else
	{
		for (FString& AttributeSetName : DataPack.AddedAttributeSets)
		{
			CanvasContext.Printf(TEXT("\t\t\t{yellow}%s"), *AttributeSetName);
		}
	}
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_Equipment::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_Equipment());
}

void FGameplayDebuggerCategory_Equipment::FRepData::Serialize(FArchive& Ar)
{
	// Data.
	Ar << EquipmentDisplayName;
	Ar << EquipmentDef;
	Ar << EquipmentInstanceType;
	Ar << AssociatedInventoryItem;
	Ar << SpawnedEquipmentActors;
	Ar << GrantedAbilities;
	Ar << AppliedEffects;
	Ar << AddedAttributeSets;

	Ar << DebugActorName;

	// Bit flags.
	uint32 BitFlags =
		((bHasEquipmentComp ? 1 : 0) << 0) |
		((bHasEquipment ? 1 : 0) << 1);

	Ar << BitFlags;

	bHasEquipmentComp = (BitFlags & (1 << 0)) != 0;
	bHasEquipment = (BitFlags & (1 << 1)) != 0;
}

#endif // WITH_GAMEPLAY_DEBUGGER