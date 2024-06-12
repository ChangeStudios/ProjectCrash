// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureManager.h"

#if WITH_EDITOR

void UGameFeatureManager::OnPlayInEditorBegin()
{
	// Clear the game feature request map.
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UGameFeatureManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		// Retrieve the current game feature manager.
		UGameFeatureManager* GameFeatureManager = GEngine->GetEngineSubsystem<UGameFeatureManager>();
		check(GameFeatureManager);

		// Add a new request for the given plugin.
		int32& Count = GameFeatureManager->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UGameFeatureManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		// Retrieve the current game feature manager.
		UGameFeatureManager* GameFeatureManager = GEngine->GetEngineSubsystem<UGameFeatureManager>();
		check(GameFeatureManager);

		// Decrement the number of active requests for the given plugin.
		int32& Count = GameFeatureManager->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;

		// If there are no more active requests for this plugin, it can be deactivated.
		if (Count == 0)
		{
			GameFeatureManager->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}

		return false;
	}

	return true;
}

#endif // WITH_EDITOR