/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmakingServers
*/

#include "SteamMatchmakingServers/SteamMatchmakingServersAsyncTasks.h"
#include "SteamCorePro/SteamUtilities.h"
#include "SteamCoreProPluginPrivatePCH.h"

#if WITH_STEAMCORE
HServerListRequest FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::m_CallbackResults = nullptr;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList(FOnServerUpdated ServerUpdateCallback, int32 AppID, float Timeout, int32 MaxResults, ESteamServerListRequestType RequestType, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
	: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
	, m_OnSteamCallback(ServerUpdateCallback)
	, m_AppID(AppID)
	, m_MaxResults(MaxResults)
	, m_RequestType(RequestType)
	, m_bIgnoreNonResponsive(bIgnoreNonResponsive)
	, m_ServerFilter(ServerFilter)
{
	m_AsyncTimeout = Timeout;
	
	if (Subsystem)
	{
		if (Subsystem->CurrentMatchmakingServersServerList != nullptr)
		{
			Subsystem->CurrentMatchmakingServersServerList->CancelServerQuery();
		}
		
		Subsystem->CurrentMatchmakingServersServerList = this;
	}
}

FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::~FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList()
{
	m_OnSteamCallback.Unbind();
	m_OnServerRefreshCompleted.Unbind();

	if (Subsystem)
	{
		if (Subsystem->CurrentMatchmakingServersServerList == this)
		{
			Subsystem->CurrentMatchmakingServersServerList = nullptr;
		}
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_FoundServers = 0;
			int32 NumFilters = 0;
			MatchMakingKeyValuePair_t* Filters = nullptr;

			if (m_ServerFilter != nullptr)
			{
				NumFilters = m_ServerFilter->GetNumFilters();
				Filters = m_ServerFilter->GetFiltersPtr();
			}

			switch (m_RequestType)
			{
			case ESteamServerListRequestType::Favorites:
				m_CallbackResults = SteamMatchmakingServers()->RequestFavoritesServerList(m_AppID, &Filters, NumFilters, this);
				break;
			case ESteamServerListRequestType::Friends:
				m_CallbackResults = SteamMatchmakingServers()->RequestFriendsServerList(m_AppID, &Filters, NumFilters, this);
				break;
			case ESteamServerListRequestType::History:
				m_CallbackResults = SteamMatchmakingServers()->RequestHistoryServerList(m_AppID, &Filters, NumFilters, this);
				break;
			case ESteamServerListRequestType::Internet:
				m_CallbackResults = SteamMatchmakingServers()->RequestInternetServerList(m_AppID, &Filters, NumFilters, this);
				break;
			case ESteamServerListRequestType::Lan:
				m_CallbackResults = SteamMatchmakingServers()->RequestLANServerList(m_AppID, this);
				break;
			case ESteamServerListRequestType::Spectator:
				m_CallbackResults = SteamMatchmakingServers()->RequestSpectatorServerList(m_AppID, &Filters, NumFilters, this);
				break;
			}

			if (m_CallbackResults == nullptr)
			{
				bIsComplete = true;
				bWasSuccessful = false;
			}

			if (m_ServerFilter != nullptr)
			{
				m_ServerFilter->SetReadyToDestroy();
			}

			bInit = true;
		}

		const bool bReachedSearchLimit = (m_FoundServers >= m_MaxResults) ? true : false;
		const bool bServerSearchComplete = (m_bServerRefreshComplete && m_ElapsedTime > 1.0f) ? true : false;

		if (bReachedSearchLimit || bTimedOut || bServerSearchComplete)
		{
			bIsComplete = true;
			bWasSuccessful = true;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}

	if (bIsComplete)
	{
		m_ServerFilter.Reset();
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::Finalize()
{
	LogSteamCoreVerbose("");

	CancelServerQuery();
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::CancelServerQuery()
{
	LogSteamCoreVerbose("");

	if (!bIsComplete)
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
	
	SteamMatchmakingServers()->CancelQuery(m_CallbackResults);
	SteamMatchmakingServers()->ReleaseRequest(m_CallbackResults);
	m_CallbackResults = nullptr;
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::ServerResponded(HServerListRequest Request, int iServer)
{
	LogSteamCoreVeryVerbose("");

	m_ElapsedTime = 0.0f;

	gameserveritem_t* Server = SteamMatchmakingServers()->GetServerDetails(Request, iServer);

	if (Server != nullptr)
	{
		if (Server->m_nAppID == m_AppID)
		{
			FString ServerName = UTF8_TO_TCHAR(Server->GetName());
			FString MapName = UTF8_TO_TCHAR(Server->m_szMap);
			LogSteamCoreVerbose("Server Responded: (%s), MapName: (%s), Players/Max: (%i/%i)", *ServerName, *MapName, Server->m_nPlayers, Server->m_nMaxPlayers);
			m_OnSteamCallback.ExecuteIfBound(FGameServerItem(Server));
			m_FoundServers++;
		}
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::ServerFailedToRespond(HServerListRequest Request, int iServer)
{
	LogSteamCoreVeryVerbose("");

	m_ElapsedTime = 0.0f;

	if (!m_bIgnoreNonResponsive)
	{
		gameserveritem_t* Server = SteamMatchmakingServers()->GetServerDetails(Request, iServer);

		if (Server != nullptr)
		{
			if (Server->m_nAppID == m_AppID)
			{
				m_OnSteamCallback.ExecuteIfBound(FGameServerItem(Server));
				m_FoundServers++;
			}
		}
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList::RefreshComplete(HServerListRequest Request, EMatchMakingServerResponse Response)
{
	m_bServerRefreshComplete = true;
	m_OnServerRefreshCompleted.ExecuteIfBound();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer::~FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer()
{
	if (Subsystem)
	{
		if (Subsystem->CurrentMatchmakingServersPingServer == this)
		{
			Subsystem->CurrentMatchmakingServersPingServer = nullptr;
		}
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			FIPv4Address NewIP;
			FIPv4Address::Parse(m_IP, NewIP);

			m_CallbackResults = SteamMatchmakingServers()->PingServer(NewIP.Value, m_Port, this);

			if (m_CallbackResults == k_uAPICallInvalid)
			{
				bIsComplete = true;
				bWasSuccessful = false;
			}

			bInit = true;
		}

		if (bIsComplete)
		{
			// Cancel further server queries (may trigger RefreshComplete delegate)
			if (m_CallbackResults != k_uAPICallInvalid)
			{
				SteamMatchmakingServers()->CancelServerQuery(m_CallbackResults);
				m_CallbackResults = k_uAPICallInvalid;
			}
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer::CancelServerQuery()
{
	LogSteamCoreVerbose("");

	if (!bIsComplete)
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
	
	SteamMatchmakingServers()->CancelServerQuery(m_CallbackResults);
	m_CallbackResults = k_uAPICallInvalid;
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer::ServerResponded(gameserveritem_t& server)
{
	LogSteamCoreVeryVerbose("");

	bIsComplete = true;
	bWasSuccessful = true;

	m_OnSteamCallback.ExecuteIfBound(&server, bWasSuccessful);
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer::ServerFailedToRespond()
{
	LogSteamCoreVeryVerbose("");

	bIsComplete = true;
	bWasSuccessful = false;

	m_OnSteamCallback.ExecuteIfBound(FGameServerItem(), bWasSuccessful);
}

FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::~FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules()
{
	if (Subsystem)
	{
		if (Subsystem->CurrentMatchmakingServersServerRules == this)
		{
			Subsystem->CurrentMatchmakingServersServerRules = nullptr;
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

HServerQuery FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::m_CallbackResults = k_uAPICallInvalid;

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			FIPv4Address NewIP;
			FIPv4Address::Parse(m_IP, NewIP);

			m_CallbackResults = SteamMatchmakingServers()->ServerRules(NewIP.Value, m_Port, this);

			if (m_CallbackResults == k_uAPICallInvalid)
			{
				bIsComplete = true;
				bWasSuccessful = false;
			}

			bInit = true;
		}

		if (bIsComplete)
		{
			// Cancel further server queries (may trigger RefreshComplete delegate)
			if (m_CallbackResults != k_uAPICallInvalid)
			{
				SteamMatchmakingServers()->CancelServerQuery(m_CallbackResults);
				m_CallbackResults = k_uAPICallInvalid;
			}
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::CancelServerQuery()
{
	LogSteamCoreVerbose("");

	if (!bIsComplete)
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
	
	SteamMatchmakingServers()->CancelServerQuery(m_CallbackResults);
	m_CallbackResults = k_uAPICallInvalid;
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::RulesResponded(const char* pchRule, const char* pchValue)
{
	LogSteamCoreVeryVerbose("");

	m_Rules.Add({pchRule, pchValue});
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::RulesFailedToRespond()
{
	LogSteamCoreVeryVerbose("");
}

void FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules::RulesRefreshComplete()
{
	LogSteamCoreVerbose("");

	bIsComplete = true;
	bWasSuccessful = true;

	m_OnSteamCallback.ExecuteIfBound(m_Rules, bWasSuccessful);
}
#endif