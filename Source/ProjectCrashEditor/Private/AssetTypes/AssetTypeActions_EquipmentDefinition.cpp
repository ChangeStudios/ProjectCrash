// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_EquipmentDefinition.h"

#include "ProjectCrashEditor.h"
#include "Equipment/EquipmentDefinition.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_EquipmentDefinition::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_EquipmentDefinition", "Equipment Definition");
}

FColor FAssetTypeActions_EquipmentDefinition::GetTypeColor() const
{
	return FColor(75, 225, 75);
}

UClass* FAssetTypeActions_EquipmentDefinition::GetSupportedClass() const
{
	return UEquipmentDefinition::StaticClass();
}

uint32 FAssetTypeActions_EquipmentDefinition::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetInventoryAssetCategory();
}

#undef LOCTEXT_NAMESPACE