// Copyright Epic Games, Inc. All Rights Reserved.

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

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreOnline",
			"CoreUObject",
			"Engine",
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
			"NetCore",
			"Slate",
			"SlateCore",
			"UIExtension",
			"UMG"
		});
	}
}
