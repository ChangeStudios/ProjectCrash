// Copyright Samuel Reitich 2024.


#include "System/CrashAssetManager.h"

#include "AbilitySystem/CrashNativeGameplayTags.h"

UCrashAssetManager::UCrashAssetManager()
{
}

UCrashAssetManager& UCrashAssetManager::Get()
{
	check(GEngine);

	// Ensure that this is being used as the engine's asset manager.
	if (UCrashAssetManager* Manager = Cast<UCrashAssetManager>(GEngine->AssetManager))
	{
		return *Manager;
	}

	UE_LOG(LogEngine, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini. Use UCrashAssetManager or the engine default."));

	// Create a new asset manager, even though it won't be the one used by the engine. The fatal error above prevents this from being called.
	return *NewObject<UCrashAssetManager>();
}

void UCrashAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// Initialize the native gameplay tags to be added to the tag manager.
	FCrashNativeGameplayTags::InitializeNativeTags();
}