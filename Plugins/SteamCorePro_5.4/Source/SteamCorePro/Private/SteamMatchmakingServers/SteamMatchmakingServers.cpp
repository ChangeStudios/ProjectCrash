/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmakingServers
*/

#include "SteamMatchmakingServers/SteamMatchmakingServers.h"
#include "SteamMatchmakingServers/SteamMatchmakingServersAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProMatchmakingServers::USteamProMatchmakingServers()
{
}

USteamProMatchmakingServers::~USteamProMatchmakingServers()
{
}

USteamProMatchmakingServers* USteamProMatchmakingServers::GetSteamMatchmakingServers()
{
#if WITH_STEAMCORE
	return SteamMatchmakingServers() ? ThisClass::StaticClass()->GetDefaultObject<USteamProMatchmakingServers>() : nullptr;
#else
	return nullptr;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


void USteamProMatchmakingServers::RequestFavoritesServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive /* = false */, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::Favorites, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::RequestFriendsServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive /* = false */, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::Friends, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::RequestHistoryServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive /* = false */, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::History, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::RequestInternetServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive /* = false */, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::Internet, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::RequestLANServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive /* = false */, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::Lan, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::RequestSpectatorServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

	RequestServerList(ServerCallback, AppID, Timeout, ESteamServerListRequestType::Spectator, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamProMatchmakingServers::PingServer(const FOnServerPing& Callback, FString IP, int32 Port)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmakingServers())
	{
		FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer* Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer(Callback, IP, Port);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProMatchmakingServers::ServerRules(const FOnServerRules& Callback, FString IP, int32 Port)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmakingServers())
	{
		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules* Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules(Callback, IP, Port);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProMatchmakingServers::RequestServerList(const FOnServerUpdated& ServerCallback, int32 AppID, float Timeout, ESteamServerListRequestType Type, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmakingServers())
	{
		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList* Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList(ServerCallback, AppID, Timeout, MaxResults, Type, bIgnoreNonResponsive, ServerFilter);
		QueueAsyncTask(Task);
	}
#endif
}
