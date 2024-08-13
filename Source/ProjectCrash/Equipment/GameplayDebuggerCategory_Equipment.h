// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

/**
 * Gameplay debugger category for the equipment system.
 */
class PROJECTCRASH_API FGameplayDebuggerCategory_Equipment : public FGameplayDebuggerCategory
{

public:

    FGameplayDebuggerCategory_Equipment();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:

	struct FRepData
	{
		/** Debug info for the equipped item. */
		FString EquipmentDisplayName;
		FString EquipmentDef;
		FString EquipmentInstanceType;
		FString AssociatedInventoryItem;
		TArray<FString> SpawnedEquipmentActors;
		TArray<FString> GrantedAbilities;
		TArray<FString> AppliedEffects;
		TArray<FString> AddedAttributeSets;

		FString DebugActorName;
		uint32 bHasEquipmentComp : 1;
		uint32 bHasEquipment : 1;

		FRepData() : bHasEquipmentComp(false), bHasEquipment(false)
		{
		}

		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER