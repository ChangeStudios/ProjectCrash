// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

class UInventoryItemInstance;

/**
 * Gameplay debugger category for the inventory system.
 */
class PROJECTCRASH_API FGameplayDebuggerCategory_Inventory : public FGameplayDebuggerCategory
{

public:

    FGameplayDebuggerCategory_Inventory();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;

	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

protected:

	struct FRepData
	{
		/** Debug info for one item. */
		struct FItem
		{
			FString DisplayName;
			FString ItemDef;
			FString ItemOwnerName;
			TMap<FString, int32> StatTags;
			TArray<FString> GrantedAbilities;
			TArray<FString> AppliedEffects;
			TArray<FString> AddedAttributeSets;
		};

		FString InventoryOwnerName;
		uint32 bHasInventory : 1;
		TArray<FItem> Items;

		FRepData() : bHasInventory(false)
		{
		}

		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER