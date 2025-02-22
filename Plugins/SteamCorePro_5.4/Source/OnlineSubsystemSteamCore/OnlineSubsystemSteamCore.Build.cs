/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

using UnrealBuildTool;
using System.IO;

public class OnlineSubsystemSteamCore : ModuleRules
{
	public OnlineSubsystemSteamCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDefinitions.Add("ONLINESUBSYSTEMSTEAMCORE_PACKAGE=1");
		PrivatePCHHeaderFile = "Private/OnlineSubsystemSteamCorePrivatePCH.h";

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"OnlineSubsystemUtils",
				"OnlineSubsystem",
				"SteamCoreShared",
				"Core",
				"CoreUObject", 
				"AudioCapture",
			}
		);
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public"),
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core", 
				"CoreUObject",
				"NetCore",
				"Engine", 
				"Sockets", 
				"Voice",
                "AudioMixer",
				"OnlineSubsystem",
				"Json",
				"PacketHandler",
				"Projects",
                "SteamCoreShared",
                "SteamLibrary",
                "Voice",
                "AudioCapture",
                "AudioCaptureCore", 
			}
		);
		
		if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 1)
		{
			PrivateDependencyModuleNames.Add("OnlineBase");
		}
	}
}
