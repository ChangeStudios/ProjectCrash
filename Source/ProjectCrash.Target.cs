// Copyright Samuel Reitich. All rights reserved.

using UnrealBuildTool;

public class ProjectCrashTarget : TargetRules
{
	public ProjectCrashTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.Add("ProjectCrash");

		ProjectCrashTarget.ApplySharedTargetSettings(this);
	}

	/** Applies settings used by all targets. */
	internal static void ApplySharedTargetSettings(TargetRules Target)
	{
		Target.DefaultBuildSettings = BuildSettingsVersion.V5;
		Target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
	}
}
