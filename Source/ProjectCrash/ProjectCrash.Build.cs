// Copyright Samuel Reitich. All rights reserved.

using UnrealBuildTool;

public class ProjectCrash : ModuleRules
{
	public ProjectCrash(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			"ProjectCrash"
		});

        PrivateIncludePaths.AddRange(new string[] 
		{
		});

        PublicDependencyModuleNames.AddRange(new string[]
		{
			"AIModule",
			"Core",
			"CoreOnline",
			"CoreUObject",
			"Engine",
			"GameFeatures",
			"GameplayAbilities",
			"GameplayMessageRuntime",
			"GameplayTags",
			"GameplayTasks",
			"InputCore",
			"ModularGameplay",
			"ModularGameplayActors",
			"Niagara",
			"PhysicsCore",
			"SteamCorePro",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AnimGraphRuntime",
			"CommonGame",
			"CommonInput",
			"CommonUI",
			"CommonUser",
			"DeveloperSettings",
			"EnhancedInput",
			"LevelSequence",
			"MovieScene",
			"NetCore",
			"Slate",
			"SlateCore",
			"UIExtension",
			"UMG",
		});

        DynamicallyLoadedModuleNames.AddRange(new string[] 
		{
		});

        // We use the editor module to check the expected number of PIE clients to wait for before starting the game.
        // @see ACrashGameState::GetNumExpectedPlayers.
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

        // Generate compile errors if using DrawDebug functions in test/shipping builds.
        PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
