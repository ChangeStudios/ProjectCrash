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
			"GameplayTags",
			"GameplayTasks",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CommonUI",
			"EnhancedInput",
			"GameplayMessageRuntime",
			"LevelSequence",
			"NetCore",
			"Slate",
			"SlateCore",
			"UMG"
		});
	}
}
