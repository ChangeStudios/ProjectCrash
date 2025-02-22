/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmakingServers
*/

#pragma once

#include "SteamCoreProModule.h"
#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamMatchmakingServersTypes.h"

class UServerFilter;

#if WITH_STEAMCORE

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList : public FOnlineAsyncTaskSteamCorePro, public ISteamMatchmakingServerListResponse
{
public:
	FOnServerUpdated m_OnSteamCallback;
	FOnServerRefreshCompleted m_OnServerRefreshCompleted;
	friend class USteamCoreProMatchmakingServersAsyncActionRequestServerList;
public:
	static HServerListRequest m_CallbackResults;
public:
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList(FOnServerUpdated ServerUpdateCallback, int32 AppID, float Timeout, int32 MaxResults, ESteamServerListRequestType RequestType, bool bIgnoreNonResponsive, UServerFilter* ServerFilter);

	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList(USteamCoreProAsyncAction* AsyncObject, int32 AppID, float Timeout, int32 MaxResults, ESteamServerListRequestType RequestType, bool bIgnoreNonResponsive, UServerFilter* ServerFilter)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_FoundServers(0)
		  , m_AppID(AppID)
		  , m_MaxResults(MaxResults)
		  , m_RequestType(RequestType)
		  , m_bServerRefreshComplete(false)
		  , m_bIgnoreNonResponsive(bIgnoreNonResponsive)
		  , m_ElapsedTime(0)
		  , m_ServerFilter(ServerFilter)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
		m_OnServerRefreshCompleted.BindUFunction(AsyncObject, "HandleServerListFinished");

		if (Subsystem)
		{
			if (Subsystem->CurrentMatchmakingServersServerList != nullptr)
			{
				Subsystem->CurrentMatchmakingServersServerList->CancelServerQuery();
			}
			Subsystem->CurrentMatchmakingServersServerList = this;
		}
	}

	virtual ~FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList() override;
private:
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList() = delete;
protected:
	int32 m_FoundServers;
	int32 m_AppID;
	int32 m_MaxResults;
	ESteamServerListRequestType m_RequestType;
	bool m_bServerRefreshComplete;
	bool m_bIgnoreNonResponsive;
	float m_ElapsedTime;
	TWeakObjectPtr<UServerFilter> m_ServerFilter;
protected:
	virtual void Tick() override;
	virtual void Finalize() override;
	void CancelServerQuery();

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList"));
	}

	virtual void ServerResponded(HServerListRequest Request, int iServer) override;
	virtual void ServerFailedToRespond(HServerListRequest Request, int iServer) override;
	virtual void RefreshComplete(HServerListRequest Request, EMatchMakingServerResponse Response) override;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer : public FOnlineAsyncTaskSteamCorePro, public ISteamMatchmakingPingResponse
{
public:
	FOnServerPing m_OnSteamCallback;
	friend class USteamCoreProMatchmakingServersAsyncActionPingServer;
public:
	HServerQuery m_CallbackResults;
public:
	FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer(FOnServerPing Callback, FString IP, int32 Port, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_IP(IP)
		  , m_Port(Port)
	{
		if (Subsystem)
		{
			if (Subsystem->CurrentMatchmakingServersPingServer != nullptr)
			{
				Subsystem->CurrentMatchmakingServersPingServer->CancelServerQuery();
			}
			
			Subsystem->CurrentMatchmakingServersPingServer = this;
		}
	}

	FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer(USteamCoreProAsyncAction* AsyncObject, FString IP, int32 Port, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_IP(IP)
		  , m_Port(Port)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");

		if (Subsystem)
		{
			if (Subsystem->CurrentMatchmakingServersPingServer != nullptr)
			{
				Subsystem->CurrentMatchmakingServersPingServer->CancelServerQuery();
			}
			Subsystem->CurrentMatchmakingServersPingServer = this;
		}
	}

private:
	virtual ~FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer() override;
	FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer() = delete;
protected:
	FString m_IP;
	int32 m_Port;
protected:
	virtual void Tick() override;
	void CancelServerQuery();

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer"));
	}

	virtual void ServerResponded(gameserveritem_t& server) override;
	virtual void ServerFailedToRespond() override;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules : public FOnlineAsyncTaskSteamCorePro, public ISteamMatchmakingRulesResponse
{
public:
	FOnServerRules m_OnSteamCallback;
	friend class USteamCoreProMatchmakingServersAsyncActionServerRules;
public:
	static HServerQuery m_CallbackResults;
public:
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules(FOnServerRules Callback, FString IP, int32 Port, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_IP(IP)
		  , m_Port(Port)
	{
		if (Subsystem)
		{
			if (Subsystem->CurrentMatchmakingServersServerRules != nullptr)
			{
				Subsystem->CurrentMatchmakingServersServerRules->CancelServerQuery();
			}
			Subsystem->CurrentMatchmakingServersServerRules = this;
		}
	}

	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules(USteamCoreProAsyncAction* AsyncObject, FString IP, int32 Port, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_IP(IP)
		  , m_Port(Port)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");

		if (Subsystem)
		{
			if (Subsystem->CurrentMatchmakingServersServerRules != nullptr)
			{
				Subsystem->CurrentMatchmakingServersServerRules->CancelServerQuery();
			}
			Subsystem->CurrentMatchmakingServersServerRules = this;
		}
	}

private:
	virtual ~FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules() override;
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules() = delete;
protected:
	FString m_IP;
	int32 m_Port;
	TArray<FGameServerRule> m_Rules;
protected:
	virtual void Tick() override;
	void CancelServerQuery();

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules"));
	}

	virtual void RulesResponded(const char* pchRule, const char* pchValue) override;
	virtual void RulesFailedToRespond() override;
	virtual void RulesRefreshComplete() override;
};
#endif