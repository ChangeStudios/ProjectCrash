// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_CrashCameraMode.h"

#include "Camera/CrashCameraModeBase.h"
#include "Factories/CrashCameraModeFactory.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_CrashCameraMode::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CrashCameraMode", "Crash Camera Mode");
}

UClass* FAssetTypeActions_CrashCameraMode::GetSupportedClass() const
{
	return UCrashCameraModeBase::StaticClass();
}

UFactory* FAssetTypeActions_CrashCameraMode::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UCrashCameraModeFactory* CrashCameraModeFactory = NewObject<UCrashCameraModeFactory>();
	return CrashCameraModeFactory;
}

uint32 FAssetTypeActions_CrashCameraMode::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

#undef LOCTEXT_NAMESPACE