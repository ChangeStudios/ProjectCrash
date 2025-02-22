/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmakingServers
*/

#include "SteamMatchmakingServers/SteamMatchmakingServersAsyncActions.h"
#include "SteamMatchmakingServers/SteamMatchmakingServersAsyncTasks.h"
#include "SteamCorePro/SteamCoreProModule.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingServersAsyncActionPingServer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProMatchmakingServersAsyncActionPingServer* USteamCoreProMatchmakingServersAsyncActionPingServer::PingServerAsync(UObject* WorldContextObject, FString IP, int32 Port, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		if (Subsystem->CurrentMatchmakingServersPingServer != nullptr)
		{
			Subsystem->CurrentMatchmakingServersPingServer->CancelServerQuery();
		}

		const auto AsyncObject = NewObject<USteamCoreProMatchmakingServersAsyncActionPingServer>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer(AsyncObject, IP, Port, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProMatchmakingServersAsyncActionPingServer::CancelPingQueries(UObject* WorldContextObject)
{
#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		Subsystem->CurrentMatchmakingServersPingServer->CancelServerQuery();
	}
#endif
}

void USteamCoreProMatchmakingServersAsyncActionPingServer::HandleCallback(const FGameServerItem& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingServersAsyncActionRequestServerList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestServerList(UObject* WorldContextObject, ESteamServerListRequestType RequestType, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		if (Subsystem->CurrentMatchmakingServersServerList != nullptr)
		{
			Subsystem->CurrentMatchmakingServersServerList->CancelServerQuery();
		}

		const auto AsyncObject = NewObject<USteamCoreProMatchmakingServersAsyncActionRequestServerList>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList(AsyncObject, AppID, Timeout, MaxResults, RequestType, bIgnoreNonResponsive, ServerFilter);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProMatchmakingServersAsyncActionRequestServerList::HandleCallback(const FGameServerItem& Data)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		OnCallback.Broadcast(Data);
	});
#endif
}

void USteamCoreProMatchmakingServersAsyncActionRequestServerList::HandleServerListFinished()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		OnRefreshCompleted.Broadcast();
		SetReadyToDestroy();
	});
#endif
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestInternetServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::Internet, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestLANServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::Lan, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestSpectatorServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::Spectator, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestFavoritesServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::Favorites, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestFriendsServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::Friends, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

USteamCoreProMatchmakingServersAsyncActionRequestServerList* USteamCoreProMatchmakingServersAsyncActionRequestServerList::RequestHistoryServerListAsync(UObject* WorldContextObject, int32 AppID, float Timeout, int32 MaxResults, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
{
	return RequestServerList(WorldContextObject, ESteamServerListRequestType::History, AppID, Timeout, MaxResults, bIgnoreNonResponsive, ServerFilter);
}

void USteamCoreProMatchmakingServersAsyncActionRequestServerList::CancelServerListQueries(UObject* WorldContextObject)
{
#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		if (Subsystem->CurrentMatchmakingServersServerList != nullptr)
		{
			Subsystem->CurrentMatchmakingServersServerList->CancelServerQuery();
		}
	}
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProMatchmakingServersAsyncActionServerRules
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProMatchmakingServersAsyncActionServerRules* USteamCoreProMatchmakingServersAsyncActionServerRules::ServerRulesAsync(UObject* WorldContextObject, FString Ip, int32 QueryPort, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		if (Subsystem->CurrentMatchmakingServersServerRules != nullptr)
		{
			Subsystem->CurrentMatchmakingServersServerRules->CancelServerQuery();
		}

		const auto AsyncObject = NewObject<USteamCoreProMatchmakingServersAsyncActionServerRules>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules(AsyncObject, Ip, QueryPort, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProMatchmakingServersAsyncActionServerRules::CancelServerRulesQueries(UObject* WorldContextObject)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		if (Subsystem->CurrentMatchmakingServersServerRules != nullptr)
		{
			Subsystem->CurrentMatchmakingServersServerRules->CancelServerQuery();
		}
	}
#endif
}

void USteamCoreProMatchmakingServersAsyncActionServerRules::HandleCallback(const TArray<FGameServerRule>& Data, bool bWasSuccessful)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	AsyncTask(ENamedThreads::GameThread, [this, Data, bWasSuccessful]()
	{
		OnCallback.Broadcast(Data, bWasSuccessful);

		SetReadyToDestroy();
	});
#endif
}
