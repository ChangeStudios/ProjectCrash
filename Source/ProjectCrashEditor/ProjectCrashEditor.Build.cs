// Copyright Samuel Reitich. All rights reserved.

using UnrealBuildTool;

public class ProjectCrashEditor : ModuleRules
{
	public ProjectCrashEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				"ProjectCrashEditor"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"EditorFramework",
				"EnhancedInput",
				"Engine",
				"ProjectCrash",
				"UnrealEd",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"Kismet",
				"InputEditor",
				"Slate",
				"SlateCore",
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		SetupGameplayDebuggerSupport(Target);
	}
}