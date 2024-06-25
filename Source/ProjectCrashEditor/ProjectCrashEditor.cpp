// Copyright Samuel Reitich. All rights reserved.

#include "ProjectCrashEditor.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetTypes/AssetTypeActions_GameModeData.h"
#include "AssetTypes/UAssetTypeActions_PawnData.h"
#include "GameFramework/GameFeatures/GameFeatureManager.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "ProjectCrashEditorModule"

DEFINE_LOG_CATEGORY(LogProjectCrashEditor);

void FProjectCrashEditorModule::StartupModule()
{
	// Bind callback to when PIE starts.
	if (!IsRunningGame())
	{
		FEditorDelegates::BeginPIE.AddRaw(this, &ThisClass::OnBeginPIE);
	}

	// Register asset categories.
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	GameDataAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("GameData")), LOCTEXT("GameDataAssetCategory", "Game Data"));

	// Register asset types.
	AssetType_GameModeData = MakeShared<FAssetTypeActions_GameModeData>();
	AssetTools.RegisterAssetTypeActions(AssetType_GameModeData.ToSharedRef());
	AssetType_UserFacingGameModeData = MakeShared<FAssetTypeActions_UserFacingGameModeData>();
	AssetTools.RegisterAssetTypeActions(AssetType_UserFacingGameModeData.ToSharedRef());

	AssetType_PawnData = MakeShared<FAssetTypeActions_PawnData>();
	AssetTools.RegisterAssetTypeActions(AssetType_PawnData.ToSharedRef());
}

void FProjectCrashEditorModule::OnBeginPIE(bool bIsSimulating)
{
	// Notify the game feature manager subsystem when a PIE session starts.
	UGameFeatureManager* GameFeatureManager = GEngine->GetEngineSubsystem<UGameFeatureManager>();
	check(GameFeatureManager);
	GameFeatureManager->OnPlayInEditorBegin();
}

void FProjectCrashEditorModule::ShutdownModule()
{
	// Unbind callbacks from when PIE starts.
    FEditorDelegates::BeginPIE.RemoveAll(this);

	// Unregister asset types.
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	IAssetTools& AssetTools = IAssetTools::Get();
	AssetTools.UnregisterAssetTypeActions(AssetType_GameModeData.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_UserFacingGameModeData.ToSharedRef());
	AssetTools.UnregisterAssetTypeActions(AssetType_PawnData.ToSharedRef());
}

IMPLEMENT_MODULE(FProjectCrashEditorModule, ProjectCrashEditor);

#undef LOCTEXT_NAMESPACE