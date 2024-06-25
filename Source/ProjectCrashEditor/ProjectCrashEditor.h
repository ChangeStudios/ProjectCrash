// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AssetTypeCategories.h"
#include "Logging/LogMacros.h"

class FAssetTypeActions_GameModeData;
class FAssetTypeActions_PawnData;
class FAssetTypeActions_UserFacingGameModeData;

DECLARE_LOG_CATEGORY_EXTERN(LogProjectCrashEditor, Log, All);

/*
 * Default editor module for this project.
 */
class FProjectCrashEditorModule : public FDefaultGameModuleImpl
{
	typedef FProjectCrashEditorModule ThisClass;

	// Initialization.

public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:

	void OnBeginPIE(bool bIsSimulating);



	// Custom asset categories.

public:

	EAssetTypeCategories::Type GetGameDataAssetCategory() const { return GameDataAssetCategory; }

protected:

	EAssetTypeCategories::Type GameDataAssetCategory = EAssetTypeCategories::None;



	// Custom asset types.

protected:

	TSharedPtr<FAssetTypeActions_GameModeData> AssetType_GameModeData;
	TSharedPtr<FAssetTypeActions_UserFacingGameModeData> AssetType_UserFacingGameModeData;
	TSharedPtr<FAssetTypeActions_PawnData> AssetType_PawnData;
};