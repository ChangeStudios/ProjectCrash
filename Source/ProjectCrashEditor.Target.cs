// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectCrashEditorTarget : TargetRules
{
	public ProjectCrashEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "ProjectCrash", "ProjectCrashEditor" });

		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		ProjectCrashTarget.ApplySharedTargetSettings(this);
	}
}
