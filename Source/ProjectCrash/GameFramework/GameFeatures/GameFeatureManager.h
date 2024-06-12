// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "GameFeatureManager.generated.h"

/**
 * Subsystem for managing the activation and deactivation of game features. Primarily used to manage deactivation of
 * game features between PIE sessions, as features are automatically deactivated between games in builds.
 */
UCLASS(MinimalAPI)
class UGameFeatureManager : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

#if WITH_EDITOR

	/** Clears the list of active plugin requests. */
	PROJECTCRASH_API void OnPlayInEditorBegin();

	/** Caches the plugin activation request for deactivation later. */
	static void NotifyOfPluginActivation(const FString PluginURL);

	/**
	 * Deactivates the given plugin. This is only used for PIE, since game features are automatically deactivated
	 * between levels in builds.
	 *
	 * @return		If the given plugin no longer has any active requests for activation. The plugin can only be
	 *				safely deactivated if this is true. Always returns true outside the editor.
	 */
	static bool RequestToDeactivatePlugin(const FString PluginURL);

#else

	// Plugins are not tracked by this class outside the editor.
	static void NotifyOfPluginActivation(const FString PluginURL) {}
	static bool RequestToDeactivatePlugin(const FString PluginURL) { return true; }

#endif

private:

	/** A map of plugins to that plugin's number of active requests for activation. Used to allow FILO plugin activation
	 * during PIE. */
	TMap<FString /* plugin URL */, int32 /* request count */> GameFeaturePluginRequestCountMap;
};
