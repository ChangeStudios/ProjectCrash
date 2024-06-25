// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_PawnData.h"

#include "ProjectCrashEditor.h"
#include "Characters/Data/PawnData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_PawnData::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_PawnData", "Pawn Data");
}

FColor FAssetTypeActions_PawnData::GetTypeColor() const
{
	return FColor(255, 64, 0);
}

UClass* FAssetTypeActions_PawnData::GetSupportedClass() const
{
	return UPawnData::StaticClass();
}

uint32 FAssetTypeActions_PawnData::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetGameDataAssetCategory();
}

#undef LOCTEXT_NAMESPACE