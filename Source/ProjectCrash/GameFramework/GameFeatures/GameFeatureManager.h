// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "GameFeatureManager.generated.h"

/**
 * Subsystem for managing the activation and deactivation of game features in PIE.
 *
 * Deactivating a game feature deactivates it for the entire game, meaning any PIE instances running on a single
 * process would ALL lose the game feature if ANY of them decided to deactivate it. This subsystem prevents the
 * deactivation of a game feature as long as there is at least one active "request" for it (i.e. a PIE instance still
 * needs it).
 */
UCLASS(MinimalAPI)
class UGameFeatureManager : public UEngineSubsystem
{
	GENERATED_BODY()

virtual void BeginDestroy() override;
	
public:

	// TODO: Implement to safely deactivate and unload game features when the game ends, since this class persists after game state.
	static void DeactivateAndUnloadGameFeaturePlugin(const FString PluginURL);

#if WITH_EDITOR

	/** Clears the list of active plugin requests. */
	PROJECTCRASH_API void OnPlayInEditorBegin();

	/** Caches the plugin activation request for deactivation later. */
	static void NotifyOfPluginActivation(const FString PluginURL);

	/** Returns if the plugin can safely be deactivated, i.e. there are no active requests for its activation. */
	static bool RequestToDeactivatePlugin(const FString PluginURL);

#else

	/* Plugins requests are not tracked by this class outside the editor. An independent game process will only ever
	 * have one request for plugin activation. */
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }

#endif

private:

	/** A map of plugins to that plugin's number of active requests for activation (i.e. PIE instances using it). */
	TMap<FString /* plugin URL */, int32 /* request count */> GameFeaturePluginRequestCountMap;
};
