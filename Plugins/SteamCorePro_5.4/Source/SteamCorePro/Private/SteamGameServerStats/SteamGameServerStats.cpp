/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamGameServerStats
*/

#include "SteamGameServerStats/SteamGameServerStats.h"
#include "SteamGameServerStats/SteamGameServerStatsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProGameServerStats::USteamProGameServerStats()
{
#if WITH_STEAMCORE
	OnGSStatsUnloadedCallback.Register(this, &USteamProGameServerStats::OnGSStatsUnloaded);

	if (IsRunningDedicatedServer())
	{
		OnGSStatsUnloadedCallback.SetGameserverFlag();
	}
#endif
}

USteamProGameServerStats::~USteamProGameServerStats()
{
#if WITH_STEAMCORE
	OnGSStatsUnloadedCallback.Unregister();
#endif
}

USteamProGameServerStats* USteamProGameServerStats::GetSteamGameServerStats()
{
#if WITH_STEAMCORE
	return SteamGameServerStats() ? ThisClass::StaticClass()->GetDefaultObject<USteamProGameServerStats>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

bool USteamProGameServerStats::GetUserStatInt(FSteamID SteamIDUser, FString Name, int32& OutData)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutData = 0;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->GetUserStat(SteamIDUser, TCHAR_TO_UTF8(*Name), &OutData);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::GetUserStatFloat(FSteamID SteamIDUser, FString Name, float& OutData)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutData = 0.0f;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->GetUserStat(SteamIDUser, TCHAR_TO_UTF8(*Name), &OutData);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::GetUserAchievement(FSteamID SteamIDUser, FString Name, bool& bAchieved)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	bAchieved = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->GetUserAchievement(SteamIDUser, TCHAR_TO_UTF8(*Name), &bAchieved);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::SetUserStatInt(FSteamID SteamIDUser, FString Name, int32 Data)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->SetUserStat(SteamIDUser, TCHAR_TO_UTF8(*Name), Data);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::SetUserStatFloat(FSteamID SteamIDUser, FString Name, float Data)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->SetUserStat(SteamIDUser, TCHAR_TO_UTF8(*Name), Data);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::UpdateUserAvgRateStat(FSteamID SteamIDUser, FString Name, float CountThisSession, float SessionLength)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->UpdateUserAvgRateStat(SteamIDUser, TCHAR_TO_UTF8(*Name), CountThisSession, SessionLength);
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::SetUserAchievement(FSteamID SteamIDUser, FString Name)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->SetUserAchievement(SteamIDUser, TCHAR_TO_UTF8(*Name));
	}
#endif

	return bResult;
}

bool USteamProGameServerStats::ClearUserAchievement(FSteamID SteamIDUser, FString Name)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		bResult = SteamGameServerStats()->ClearUserAchievement(SteamIDUser, TCHAR_TO_UTF8(*Name));
	}
#endif

	return bResult;
}

void USteamProGameServerStats::ServerRequestUserStats(const FOnServerRequestUserStats& Callback, FSteamID SteamIDUser)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS* Task = new FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS(Callback, SteamIDUser);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProGameServerStats::ServerStoreUserStats(const FOnServerStoreUserStats& Callback, FSteamID SteamIDUser)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServerStats())
	{
		FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats* Task = new FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats(Callback, SteamIDUser);
		QueueAsyncTask(Task);
	}
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProGameServerStats::OnGSStatsUnloaded(GSStatsUnloaded_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSStatsUnloaded.Broadcast(Data);
	});
}
#endif
