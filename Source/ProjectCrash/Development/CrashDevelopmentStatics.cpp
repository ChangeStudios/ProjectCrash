// Copyright Samuel Reitich. All rights reserved.


#include "Development/CrashDevelopmentStatics.h"

#include "AssetRegistry/AssetRegistryModule.h"

UWorld* UCrashDevelopmentStatics::FindPlayInEditorAuthorityWorld()
{
	check(GEngine);

	UWorld* ServerWorld = nullptr;

#if WITH_EDITOR
	// Find the highest-authority world; ideally a world running as a dedicated server.
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.WorldType == EWorldType::PIE)
		{
			if (UWorld* World = WorldContext.World())
			{
				// Ideal case: stop searching as soon as a dedicated server world is found.
				if (WorldContext.RunAsDedicated)
				{
					ServerWorld = World;
					break;
				}
				// Ensure we always get at least one world.
				else if (ServerWorld == nullptr)
				{
					ServerWorld = World;
				}
				// Try to take the highest-authority world.
				else
				{
					if (World->GetNetMode() < ServerWorld->GetNetMode())
					{
						ServerWorld = World;
					}
				}
			}
		}
	}
#endif // WITH_EDITOR

	return ServerWorld;
}

UClass* UCrashDevelopmentStatics::FindClassByName(const FString& SearchToken, UClass* DesiredBaseClass)
{
	check(DesiredBaseClass);

	FString TargetName = SearchToken;

	// Check native classes and loaded assets first before resorting to the asset registry.
	bool bIsValidClassName = true;
	if (TargetName.IsEmpty() || TargetName.Contains(TEXT(" ")))
	{
		bIsValidClassName = false;
	}
	else if (!FPackageName::IsShortPackageName(TargetName))
	{
		if (TargetName.Contains(TEXT(".")))
		{
			// Convert ClassName'ObjectPath' to ObjectPath (will return the full string if it doesn't have ' in it).
			TargetName = FPackageName::ExportTextPathToObjectPath(TargetName);

			FString PackageName;
			FString ObjectName;
			TargetName.Split(TEXT("."), &PackageName, &ObjectName);

			const bool bIncludeReadOnlyRoots = true;
			FText Reason;
			if (!FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots, &Reason))
			{
				bIsValidClassName = false;
			}
		}
		else
		{
			bIsValidClassName = false;
		}
	}

	UClass* ResultClass = nullptr;
	if (bIsValidClassName)
	{
		ResultClass = UClass::TryFindTypeSlow<UClass>(TargetName);
	}

	// If we still haven't found anything yet, try the asset registry for blueprints that match the requirements.
	if (ResultClass == nullptr)
	{
		ResultClass = FindBlueprintClass(TargetName, DesiredBaseClass);
	}

	// Validate the class, if we have one.
	if (ResultClass != nullptr)
	{
		if (!ResultClass->IsChildOf(DesiredBaseClass))
		{
			UE_LOG(LogConsoleResponse, Warning, TEXT("Found an asset [%s] but it wasn't of type [%s]."), *ResultClass->GetPathName(), *DesiredBaseClass->GetName());
			ResultClass = nullptr;
		}
	}
	else
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("Failed to find class of type [%s] named [%s]."), *DesiredBaseClass->GetName(), *SearchToken);
	}

	return ResultClass;
}

TArray<FAssetData> UCrashDevelopmentStatics::GetAllBlueprints()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// Get all blueprints in the asset registry.
	TArray<FAssetData> BlueprintList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

	return BlueprintList;
}

UClass* UCrashDevelopmentStatics::FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass)
{
	// Remove class suffix.
	FString TargetName = TargetNameRaw;
	TargetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

	// Search all assets for a blueprint matching the given name and class.
	TArray<FAssetData> BlueprintList = GetAllBlueprints();
	for (const FAssetData& AssetData : BlueprintList)
	{
		if ((AssetData.AssetName.ToString() == TargetName) || (AssetData.GetObjectPathString() == TargetName))
		{
			if (UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
			{
				if (UClass* GeneratedClass = BP->GeneratedClass)
				{
					if (GeneratedClass->IsChildOf(DesiredBaseClass))
					{
						return GeneratedClass;
					}
				}
			}
		}
	}

	return nullptr;
}
