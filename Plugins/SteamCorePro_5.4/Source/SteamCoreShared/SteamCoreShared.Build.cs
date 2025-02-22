/**
* Copyright (C) 2017-2024 eelDev AB
*/

using UnrealBuildTool;
using System.IO;

public class SteamCoreShared : ModuleRules
{
	public SteamCoreShared(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange
		(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"SteamLibrary",
				"AudioCapture",
				"AudioCaptureCore",
			}
		);

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDefinitions.Add("STEAMCORESHARED_PACKAGE=1");
	
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core", 
				"Sockets",
				"SteamLibrary",
				"Projects", 
				"AudioCaptureCore",
			}
		);
	}
}