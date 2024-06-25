// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_InputActionMapping.h"

#include "InputEditorModule.h"
#include "Input/CrashInputActionMapping.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_InputActionMapping::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_InputActionMapping", "Input Action Mapping");
}

FColor FAssetTypeActions_InputActionMapping::GetTypeColor() const
{
	return FColor(255, 205, 127);
}

UClass* FAssetTypeActions_InputActionMapping::GetSupportedClass() const
{
	return UCrashInputActionMapping::StaticClass();
}

uint32 FAssetTypeActions_InputActionMapping::GetCategories()
{
	// TODO: Get this category working. Right now, it causes a linking error, even though we have InputEditor in our build.cs.
	// return FModuleManager::Get().GetModuleChecked<FInputEditorModule>("InputEditor").GetInputAssetsCategory();

	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE