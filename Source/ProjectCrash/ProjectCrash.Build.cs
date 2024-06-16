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
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
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
			"UnrealEd"
		});

        DynamicallyLoadedModuleNames.AddRange(new string[] 
		{
		});

        // Generate compile errors if using DrawDebug functions in test/shipping builds.
        PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

        SetupGameplayDebuggerSupport(Target);
    }
}
