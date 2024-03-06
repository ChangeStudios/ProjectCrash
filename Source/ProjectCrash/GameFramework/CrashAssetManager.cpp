// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashAssetManager.h"

#include "UnrealEngine.h"
#include "Engine/Engine.h"
#include "GameModes/Game/CrashGameModeData.h"
#include "UI/UserInterfaceData.h"

UCrashAssetManager::UCrashAssetManager()
{
}

UCrashAssetManager& UCrashAssetManager::Get()
{
	check(GEngine);

	// Retrieve the world's asset manager.
	if (UCrashAssetManager* AssetManager = Cast<UCrashAssetManager>(GEngine->AssetManager))
	{
		return *AssetManager;
	}

	UE_LOG(LogEngine, Fatal, TEXT("Project default asset manager is not set to UCrashAssetManager."));

	// Fallback for safety. This should never happen.
	return *NewObject<UCrashAssetManager>();
}

const UCrashGameModeData& UCrashAssetManager::GetGameModeData() const
{
	return *NewObject<UCrashGameModeData>();
}

const UUserInterfaceData& UCrashAssetManager::GetUIData() const
{
	return *NewObject<UUserInterfaceData>();
}

#if WITH_EDITOR
void UCrashAssetManager::DumpLoadedAssets()
{
}
#endif // WITH_EDITOR