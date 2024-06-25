// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AssetTypeCategories.h"
#include "Logging/LogMacros.h"

class FAssetTypeActions_ActionSet;
class FAssetTypeActions_CrashAbilitySet;
class FAssetTypeActions_CrashCameraMode;
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

	TSharedPtr<FAssetTypeActions_ActionSet> AssetType_ActionSet;
	TSharedPtr<FAssetTypeActions_CrashCameraMode> AssetType_CrashCameraMode;
	TSharedPtr<FAssetTypeActions_CrashAbilitySet> AssetType_CrashAbilitySet;
	TSharedPtr<FAssetTypeActions_GameModeData> AssetType_GameModeData;
	TSharedPtr<FAssetTypeActions_PawnData> AssetType_PawnData;
	TSharedPtr<FAssetTypeActions_UserFacingGameModeData> AssetType_UserFacingGameModeData;



	// Custom asset type icons.

protected:

	static inline TSharedPtr<FSlateStyleSet> StyleSetInstance = nullptr;

	const FVector2D Icon16x16 = FVector2D(16.0f, 16.0f);
	const FVector2D Icon64x64 = FVector2D(64.0f, 64.0f);
};