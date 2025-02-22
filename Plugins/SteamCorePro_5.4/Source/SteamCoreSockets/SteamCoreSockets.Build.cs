/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

using UnrealBuildTool;
using System.IO;

public class SteamCoreSockets : ModuleRules
{
    public SteamCoreSockets(ReadOnlyTargetRules Target) : base(Target)
    {
		PublicDefinitions.Add("STEAMCORESOCKETS_MODULE=1");

		PublicDependencyModuleNames.AddRange(new string[] { "SteamCoreShared" });
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core", 
				"CoreUObject",
				"NetCore",
				"Engine", 
				"Sockets",
				"OnlineSubsystem",
				"PacketHandler",
                "SteamCoreShared",
                "SteamLibrary", 
                "OnlineSubsystemSteamCore"
			}
		);
	}
}
