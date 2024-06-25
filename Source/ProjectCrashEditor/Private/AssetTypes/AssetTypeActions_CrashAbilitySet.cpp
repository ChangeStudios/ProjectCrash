// Copyright Samuel Reitich. All rights reserved.


#include "AssetTypes/AssetTypeActions_CrashAbilitySet.h"

#include "AbilitySystem/Abilities/CrashAbilitySet.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FAssetTypeActions_CrashAbilitySet::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CrashAbilitySet", "Ability Set");
}

FColor FAssetTypeActions_CrashAbilitySet::GetTypeColor() const
{
	return FColor(0, 96, 128);
}

UClass* FAssetTypeActions_CrashAbilitySet::GetSupportedClass() const
{
	return UCrashAbilitySet::StaticClass();
}

uint32 FAssetTypeActions_CrashAbilitySet::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

#undef LOCTEXT_NAMESPACE