// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_Teams.h"

#include "GameFramework/Teams/TeamCreationComponent.h"
#include "GameFramework/Teams/TeamDisplayAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

/**
 * FAssetTypeActions_TeamCreationComponent
 */
FText FAssetTypeActions_TeamCreationComponent::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TeamCreationComponent", "Team Creation Component");
}

FColor FAssetTypeActions_TeamCreationComponent::GetTypeColor() const
{
	return FColor(0, 0, 255);
}

UClass* FAssetTypeActions_TeamCreationComponent::GetSupportedClass() const
{
	return UTeamCreationComponent::StaticClass();
}

uint32 FAssetTypeActions_TeamCreationComponent::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}


/**
 * FAssetTypeActions_TeamDisplayAsset
 */
FText FAssetTypeActions_TeamDisplayAsset::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_TeamDisplayAsset", "Team Display Asset");
}

FColor FAssetTypeActions_TeamDisplayAsset::GetTypeColor() const
{
	return FColor(0, 255, 255);
}

UClass* FAssetTypeActions_TeamDisplayAsset::GetSupportedClass() const
{
	return UTeamDisplayAsset::StaticClass();
}

uint32 FAssetTypeActions_TeamDisplayAsset::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

#undef LOCTEXT_NAMESPACE