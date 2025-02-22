/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamAppList
*/

#pragma once

namespace SteamCoreSharedStatics
{
	static FString GetEngineIniPath()
	{
		FString Result;
		FString DefaultEngineIni = FPaths::ProjectConfigDir() / "DefaultEngine.ini";
		
#if UE_VERSION_NEWER_THAN(5,0,3)
		Result = FConfigCacheIni::NormalizeConfigIniPath(FString::Printf(TEXT("%sDefault%s.ini"), *FPaths::SourceConfigDir(), TEXT("Engine")));
#else
		Result = DefaultEngineIni;
#endif

		return Result;
	}

	static FString GetGameIniPath()
	{
		FString Result;
		FString DefaultGameIni = FPaths::ProjectConfigDir() / "DefaultGame.ini";
		
#if UE_VERSION_NEWER_THAN(5,0,3)
		Result = FConfigCacheIni::NormalizeConfigIniPath(FString::Printf(TEXT("%sDefault%s.ini"), *FPaths::SourceConfigDir(), TEXT("Game")));
#else
		Result = DefaultGameIni;
#endif

		return Result;
	}
}
