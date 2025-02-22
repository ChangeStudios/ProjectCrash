// Copyright (C) 2017-2024 eelDev AB

using UnrealBuildTool;

public class SteamCorePro : ModuleRules
{
	public SteamCorePro(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateIncludePaths.Add("SteamCorePro/Public/SteamCorePro");
		PrivateIncludePaths.Add("SteamCorePro/Private/SteamCorePro");
		PrivatePCHHeaderFile = "Private/SteamCorePro/SteamCoreProPluginPrivatePCH.h";
		PrivateDefinitions.Add("ONLINESUBSYSTEMSTEAMCORE_PACKAGE=1");

		PublicDependencyModuleNames.AddRange
		(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Networking",
				"Sockets",
				"Projects",
				"SteamLibrary",
				"SteamCoreShared",
				"AudioExtensions",
			}
		);

		if (Target.Version.MajorVersion > 4)
		{
			PublicDependencyModuleNames.AddRange
			(
				new[]
				{
					"CoreOnline"
				}
			);	
		}

		PrivateDependencyModuleNames.AddRange
		(
			new[]
			{
				"HTTP", 
				"OnlineSubsystemSteamCore",
				"AudioExtensions", 
				"AudioCapture"
			}
		);
		
		if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion >= 2)
		{
			PrivateDependencyModuleNames.AddRange
			(
				new[]
				{
					"AudioExtensions"
				}
			);
		}

		PublicDependencyModuleNames.Add("DeveloperSettings");
	}
}