// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_ActionSet.h"

#include "ProjectCrashEditor.h"
#include "GameFramework/GameFeatures/GameFeatureActionSet.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_ActionSet::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ActionSet", "Game Feature Action Set");
}

FColor FAssetTypeActions_ActionSet::GetTypeColor() const
{
	return FColor(0, 0, 128);
}

UClass* FAssetTypeActions_ActionSet::GetSupportedClass() const
{
	return UGameFeatureActionSet::StaticClass();
}

uint32 FAssetTypeActions_ActionSet::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetGameDataAssetCategory();
}

#undef LOCTEXT_NAMESPACE