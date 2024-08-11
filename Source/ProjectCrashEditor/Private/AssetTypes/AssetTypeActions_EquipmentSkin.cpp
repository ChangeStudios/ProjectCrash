// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_EquipmentSkin.h"

#include "ProjectCrashEditor.h"
#include "Equipment/EquipmentSkin.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_EquipmentSkin::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_EquipmentSkin", "Equipment Skin");
}

FColor FAssetTypeActions_EquipmentSkin::GetTypeColor() const
{
	return FColor(255, 25, 150);
}

UClass* FAssetTypeActions_EquipmentSkin::GetSupportedClass() const
{
	return UEquipmentSkin::StaticClass();
}

uint32 FAssetTypeActions_EquipmentSkin::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetSkinAssetCategory();
}

#undef LOCTEXT_NAMESPACE