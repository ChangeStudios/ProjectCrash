// Copyright Samuel Reitich. All rights reserved.

#include "AssetTypes/AssetTypeActions_GameModeData.h"

#include "ProjectCrashEditor.h"
#include "GameFramework/GameModes/CrashGameModeData.h"
#include "GameFramework/GameModes/UserFacingGameModeData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

/**
 * FAssetTypeActions_GameModeData
 */
FText FAssetTypeActions_GameModeData::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GameModeData", "Game Mode Data");
}

FColor FAssetTypeActions_GameModeData::GetTypeColor() const
{
	return FColor(0, 0, 128);
}

UClass* FAssetTypeActions_GameModeData::GetSupportedClass() const
{
	return UCrashGameModeData::StaticClass();
}

uint32 FAssetTypeActions_GameModeData::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetGameDataAssetCategory();
}



/**
 * FAssetTypeActions_UserFacingGameMode
 */
FText FAssetTypeActions_UserFacingGameModeData::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GameModeData", "User-Facing Game Mode Data");
}

FColor FAssetTypeActions_UserFacingGameModeData::GetTypeColor() const
{
	return FColor(0, 128, 255);
}

UClass* FAssetTypeActions_UserFacingGameModeData::GetSupportedClass() const
{
	return UUserFacingGameModeData::StaticClass();
}

uint32 FAssetTypeActions_UserFacingGameModeData::GetCategories()
{
	return FModuleManager::GetModuleChecked<FProjectCrashEditorModule>("ProjectCrashEditor").GetGameDataAssetCategory();
}

#undef LOCTEXT_NAMESPACE
