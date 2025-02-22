/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Containers/IndirectArray.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterfaceSteamCore.h"
#include "OnlineAsyncTaskManager.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCorePackage.h"

#define SEARCH_STEAM_HOSTIP TEXT("SteamHostIp")

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreCreateServer : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreCreateServer() = delete;
public:
	FOnlineAsyncTaskSteamCoreCreateServer(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_SessionName(InSessionName)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
private:
	bool m_bInit;
	FName m_SessionName;
};

class FOnlineAsyncTaskSteamCoreUpdateServer : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreUpdateServer()
		: m_SessionName(NAME_None),
		  m_bUpdateOnlineData(false)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreUpdateServer(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName, bool bInUpdateOnlineData, const FOnlineSessionSettings& InNewSessionSettings)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_SessionName(InSessionName),
		  m_NewSessionSettings(InNewSessionSettings),
		  m_bUpdateOnlineData(bInUpdateOnlineData)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

private:
	FName m_SessionName;
	FOnlineSessionSettings m_NewSessionSettings;
	bool m_bUpdateOnlineData;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreLogoffServer : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreLogoffServer()
		: m_bInit(false),
		  m_SessionName(NAME_None)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreLogoffServer(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_SessionName(InSessionName)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;

private:
	bool m_bInit;
	FName m_SessionName;
};

class ONLINESUBSYSTEMSTEAMCORE_API FPendingSearchResultSteamCore final : public ISteamMatchmakingRulesResponse
{
	FPendingSearchResultSteamCore() = delete;

PACKAGE_SCOPE:
	bool FillSessionFromServerRules();
	void RemoveSelf() const;

public:
	FPendingSearchResultSteamCore(class FOnlineAsyncTaskSteamCoreFindServerBase* InParentQuery)
		: m_ParentQuery(InParentQuery),
		  m_ServerQueryHandle(HSERVERQUERY_INVALID),
		  m_ServerId(FUniqueNetIdSteam::EmptyId())
	{
	}

	~FPendingSearchResultSteamCore()
	{
		CancelQuery();
	}

	virtual void RulesResponded(const char* pchRule, const char* pchValue) override;
	virtual void RulesFailedToRespond() override;
	virtual void RulesRefreshComplete() override;
	void CancelQuery();

PACKAGE_SCOPE:
	FOnlineAsyncTaskSteamCoreFindServerBase* m_ParentQuery;
	HServerQuery m_ServerQueryHandle;
	FUniqueNetIdSteamRef m_ServerId;
	TSharedPtr<FInternetAddr> m_HostAddr;
	FSteamSessionKeyValuePairs m_ServerRules;
	FOnlineSessionSearchResult m_PendingSearchResult;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindServerBase : public FOnlineAsyncTaskSteamCore, public ISteamMatchmakingServerListResponse
{
	FOnlineAsyncTaskSteamCoreFindServerBase()
		: m_ElapsedTime(0.0f),
		  m_SearchSettings(nullptr),
		  m_ServerListRequestHandle(nullptr),
		  m_bInit(false),
		  m_bServerRefreshComplete(false),
		  m_SteamMatchmakingServersPtr(nullptr)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreFindServerBase(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<class FOnlineSessionSearch>& InSearchSettings)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_ElapsedTime(0.0f),
		  m_SearchSettings(InSearchSettings),
		  m_ServerListRequestHandle(nullptr),
		  m_bInit(false),
		  m_bServerRefreshComplete(false),
		  m_SteamMatchmakingServersPtr(nullptr)
	{
	}

	virtual void CreateQuery(MatchMakingKeyValuePair_t** OutFilter, int32& NumFilters);
	void ParseSearchResult(class gameserveritem_t* ServerDetails);
	virtual void Tick() override;
	virtual void Finalize() override;

	virtual void ServerResponded(HServerListRequest Request, int iServer) override;
	virtual void ServerFailedToRespond(HServerListRequest Request, int iServer) override;
	virtual void RefreshComplete(HServerListRequest Request, EMatchMakingServerResponse Response) override;

PACKAGE_SCOPE:
	float m_ElapsedTime;
	TIndirectArray<FPendingSearchResultSteamCore> m_PendingSearchResults;
	TSharedPtr<FOnlineSessionSearch> m_SearchSettings;
	HServerListRequest m_ServerListRequestHandle;

private:
	bool m_bInit;
	bool m_bServerRefreshComplete;
	ISteamMatchmakingServers* m_SteamMatchmakingServersPtr;
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnAsyncFindServerInviteCompleteWithNetId, const bool, const int32, FUniqueNetIdPtr, const class FOnlineSessionSearchResult&);
typedef FOnAsyncFindServerInviteCompleteWithNetId::FDelegate FOnAsyncFindServerInviteCompleteWithNetIdDelegate;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindServerForInviteSession : public FOnlineAsyncTaskSteamCoreFindServerBase
{
public:
	FOnlineAsyncTaskSteamCoreFindServerForInviteSession(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<class FOnlineSessionSearch>& InSearchSettings, int32 InLocalUserNum, FOnAsyncFindServerInviteCompleteWithNetId& InDelegates)
		: FOnlineAsyncTaskSteamCoreFindServerBase(InSubsystem, InSearchSettings)
		  , m_LocalUserNum(InLocalUserNum)
		  , m_FindServerInviteCompleteWithUserIdDelegates(InDelegates)
	{
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;

protected:
	int32 m_LocalUserNum;
	FOnAsyncFindServerInviteCompleteWithNetId m_FindServerInviteCompleteWithUserIdDelegates;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindServerForFriendSession : public FOnlineAsyncTaskSteamCoreFindServerBase
{
public:
	FOnlineAsyncTaskSteamCoreFindServerForFriendSession(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<class FOnlineSessionSearch>& InSearchSettings, int32 InLocalUserNum, FOnFindFriendSessionComplete& InDelegates)
		: FOnlineAsyncTaskSteamCoreFindServerBase(InSubsystem, InSearchSettings)
		  , m_LocalUserNum(InLocalUserNum)
		  , m_FindServerInviteCompleteDelegates(InDelegates)
	{
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;

protected:
	int32 m_LocalUserNum;
	FOnFindFriendSessionComplete m_FindServerInviteCompleteDelegates;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAsyncFindServersComplete, bool);
typedef FOnAsyncFindServersComplete::FDelegate FOnAsyncFindServersCompleteDelegate;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindServers : public FOnlineAsyncTaskSteamCoreFindServerBase
{
public:
	FOnlineAsyncTaskSteamCoreFindServers(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<class FOnlineSessionSearch>& InSearchSettings, FOnAsyncFindServersComplete& InDelegates)
		: FOnlineAsyncTaskSteamCoreFindServerBase(InSubsystem, InSearchSettings),
		  m_FindServersCompleteDelegates(InDelegates)
	{
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;

protected:
	FOnAsyncFindServersComplete m_FindServersCompleteDelegates;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamCoreInviteAccepted : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamCoreInviteAccepted() = delete;

public:
	FOnlineAsyncEventSteamCoreInviteAccepted(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InFriendId, const FString& InConnectionURL)
		: FOnlineAsyncEvent(InSubsystem),
		  m_FriendId(FUniqueNetIdSteam::EmptyId()),
		  m_ConnectionURL(InConnectionURL),
		  m_LocalUserNum(0)
	{
	}

	virtual ~FOnlineAsyncEventSteamCoreInviteAccepted() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamInviteAccepted Friend: %s URL: %s"), *m_FriendId->ToDebugString(), *m_ConnectionURL);
	}

	virtual void Finalize() override;

private:
	FUniqueNetIdSteamRef m_FriendId;
	FString m_ConnectionURL;
	int32 m_LocalUserNum;
};
#endif