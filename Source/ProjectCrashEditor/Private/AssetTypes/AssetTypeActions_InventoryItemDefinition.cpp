// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_InventoryItemDefinition.h"

#include "ProjectCrashEditor.h"
#include "Editors/InventoryItemDefinitionEditor.h"
#include "Factories/InventoryItemDefinitionFactory.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Inventory/InventoryItemDefinitionBlueprint.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_InventoryItemDefinition::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_InventoryItemDefinition", "Inventory Item Definition");
}

FColor FAssetTypeActions_InventoryItemDefinition::GetTypeColor() const
{
	return FColor(25, 150, 25);
}

UClass* FAssetTypeActions_InventoryItemDefinition::GetSupportedClass() const
{
	return UInventoryItemDefinitionBlueprint::StaticClass();
}

void FAssetTypeActions_InventoryItemDefinition::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Blueprint = Cast<UBlueprint>(*ObjIt);
		if (Blueprint )
		{
			bool bLetOpen = true;
			if (!Blueprint->ParentClass)
			{
				bLetOpen = EAppReturnType::Yes == FMessageDialog::Open(
					EAppMsgType::YesNo, 
					LOCTEXT("FailedToLoadItemBlueprintWithContinue", "Inventory Item Definition Blueprint could not be loaded because it derives from an invalid class.  Check to make sure the parent class for this blueprint hasn't been removed! Do you want to continue (it can crash the editor)?")
				);
			}
		
			if (bLetOpen)
			{
				TSharedRef< FInventoryItemDefinitionEditor > NewEditor(new FInventoryItemDefinitionEditor());

				TArray<UBlueprint*> Blueprints;
				Blueprints.Add(Blueprint);

				NewEditor->InitItemEditor(Mode, EditWithinLevelEditor, Blueprints, true);
			}
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadItemBlueprint", "Inventory Item Definition Blueprint could not be loaded because it derives from an invalid class.  Check to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}
}

uint32 FAssetTypeActions_InventoryItemDefinition::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetInventoryAssetCategory();
}

UFactory* FAssetTypeActions_InventoryItemDefinition::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UInventoryItemDefinitionFactory* ItemDefinitionFactory = NewObject<UInventoryItemDefinitionFactory>();
	ItemDefinitionFactory->ItemDefinitionClass = UInventoryItemDefinition::StaticClass();
	return ItemDefinitionFactory;
}

#undef LOCTEXT_NAMESPACE
