/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineAsyncTaskManager.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreCreateLobby : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreCreateLobby()
		: m_CallbackResults(),
		  m_bInit(false),
		  m_SessionName(NAME_None),
		  m_LobbyType(k_ELobbyTypePublic), m_MaxLobbyMembers(0)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreCreateLobby(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName, ELobbyType InLobbyType, int32 InMaxLobbyMembers)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_CallbackResults(),
		  m_bInit(false),
		  m_SessionName(InSessionName),
		  m_LobbyType(InLobbyType), m_MaxLobbyMembers(InMaxLobbyMembers)
	{
	}


	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

PACKAGE_SCOPE:
	LobbyCreated_t m_CallbackResults;
private:
	bool m_bInit;
	FName m_SessionName;
	ELobbyType m_LobbyType;
	int32 m_MaxLobbyMembers;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreUpdateLobby : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreUpdateLobby()
		: m_SessionName(NAME_None),
		  m_bUpdateOnlineData(false)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreUpdateLobby(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName, bool bInUpdateOnlineData, const FOnlineSessionSettings& InNewSessionSettings)
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

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreJoinLobby : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreJoinLobby() = delete;

PACKAGE_SCOPE:
	LobbyEnter_t m_CallbackResults;

public:
	FOnlineAsyncTaskSteamCoreJoinLobby(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName, const FUniqueNetIdSteam& InLobbyId)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_CallbackResults(),
		  m_bInit(false),
		  m_SessionName(InSessionName), m_LobbyId(InLobbyId.AsShared())
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bInit;
	FName m_SessionName;
	FUniqueNetIdSteamRef m_LobbyId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreLeaveLobby : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreLeaveLobby() = delete;

public:
	FOnlineAsyncTaskSteamCoreLeaveLobby(class FOnlineSubsystemSteamCore* InSubsystem, FName InSessionName, const FUniqueNetIdSteam& InLobbyId)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_SessionName(InSessionName),
		  m_LobbyId(InLobbyId.AsShared())
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;

private:
	FName m_SessionName;
	FUniqueNetIdSteamRef m_LobbyId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindLobbiesBase : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreFindLobbiesBase()
		: m_FindLobbiesState(EFindLobbiesState::Init), m_CallbackResults(), m_SteamMatchmakingPtr(nullptr)
	{
	}

	virtual void CreateQuery();

public:
	FOnlineAsyncTaskSteamCoreFindLobbiesBase(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<FOnlineSessionSearch>& InSearchSettings)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_SearchSettings(InSearchSettings),
		  m_FindLobbiesState(EFindLobbiesState::Init),
		  m_CallbackResults(), m_SteamMatchmakingPtr(SteamMatchmaking())
	{
	}

PACKAGE_SCOPE:
	enum class EFindLobbiesState : uint8
	{
		Init,
		RequestLobbyList,
		RequestLobbyData,
		WaitForRequestLobbyData,
		Finished
	};

	TSharedPtr<class FOnlineSessionSearch> m_SearchSettings;
	EFindLobbiesState m_FindLobbiesState;
	LobbyMatchList_t m_CallbackResults;
	TArray<CSteamID> m_LobbyIDs;

	void ParseSearchResult(const FUniqueNetIdSteam& LobbyId) const;
	virtual void Tick() override;
	virtual void Finalize() override;

private:
	ISteamMatchmaking* m_SteamMatchmakingPtr;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindLobbies : public FOnlineAsyncTaskSteamCoreFindLobbiesBase
{
public:
	FOnlineAsyncTaskSteamCoreFindLobbies(class FOnlineSubsystemSteamCore* InSubsystem, const TSharedPtr<FOnlineSessionSearch>& InSearchSettings)
		: FOnlineAsyncTaskSteamCoreFindLobbiesBase(InSubsystem, InSearchSettings)
	{
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnAsyncFindLobbyCompleteWithNetId, const bool, const int32, FUniqueNetIdPtr, const class FOnlineSessionSearchResult&);
typedef FOnAsyncFindLobbyCompleteWithNetId::FDelegate FOnAsyncFindLobbyCompleteDelegateWithNetId;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession : public FOnlineAsyncTaskSteamCoreFindLobbiesBase
{
public:
	FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteamRef& FriendId, const FUniqueNetIdSteam& InLobbyId, const TSharedPtr<FOnlineSessionSearch>& InSearchSettings, int32 InLocalUserNum, const FOnAsyncFindLobbyCompleteWithNetId& InOnFindLobbyCompleteDelegates)
		: FOnlineAsyncTaskSteamCoreFindLobbiesBase(InSubsystem, InSearchSettings),
		  m_LocalUserNum(InLocalUserNum),
		  m_FriendId(FriendId),
		  m_OnFindLobbyCompleteWithNetIdDelegate(InOnFindLobbyCompleteDelegates)
	{
		m_LobbyIDs.Add(CSteamID(*(uint64*)InLobbyId.GetBytes()));
		m_FindLobbiesState = FOnlineAsyncTaskSteamCoreFindLobbiesBase::EFindLobbiesState::RequestLobbyData;
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;

private:
	int32 m_LocalUserNum;
	FUniqueNetIdSteamRef m_FriendId;
	FOnAsyncFindLobbyCompleteWithNetId m_OnFindLobbyCompleteWithNetIdDelegate;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession : public FOnlineAsyncTaskSteamCoreFindLobbiesBase
{
public:
	FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InLobbyId, const TSharedPtr<FOnlineSessionSearch>& InSearchSettings, int32 InLocalUserNum, const FOnFindFriendSessionComplete& InOnFindFriendSessionCompleteDelegate)
		: FOnlineAsyncTaskSteamCoreFindLobbiesBase(InSubsystem, InSearchSettings),
		  m_LocalUserNum(InLocalUserNum),
		  m_OnFindFriendSessionCompleteDelegate(InOnFindFriendSessionCompleteDelegate)
	{
		m_LobbyIDs.Add(CSteamID(*(uint64*)InLobbyId.GetBytes()));
		m_FindLobbiesState = FOnlineAsyncTaskSteamCoreFindLobbiesBase::EFindLobbiesState::RequestLobbyData;
	}

	virtual FString ToString() const override;
	virtual void TriggerDelegates() override;

private:
	int32 m_LocalUserNum;
	FOnFindFriendSessionComplete m_OnFindFriendSessionCompleteDelegate;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamCoreLobbyInviteAccepted : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamCoreLobbyInviteAccepted() = delete;

public:
	FOnlineAsyncEventSteamCoreLobbyInviteAccepted(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InFriendId, const FUniqueNetIdSteam& InLobbyId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_FriendId(InFriendId.AsShared()),
		  m_LobbyId(InLobbyId.AsShared()),
		  m_LocalUserNum(0)
	{
	}

	virtual ~FOnlineAsyncEventSteamCoreLobbyInviteAccepted() override
	{
	}

	virtual FString ToString() const override;
	virtual void Finalize() override;

private:
	FUniqueNetIdSteamRef m_FriendId;
	FUniqueNetIdSteamRef m_LobbyId;
	int32 m_LocalUserNum;
};

namespace SteamCore
{
	bool FillSessionFromLobbyData(const FOnlineSubsystemSteamCore* SteamSubsystem, const FUniqueNetIdSteam& LobbyId, class FOnlineSession& Session, FOnlineSessionSearchResult* SearchData = nullptr);
	bool FillMembersFromLobbyData(const FUniqueNetIdSteam& LobbyId, class FNamedOnlineSession& Session);
}
#endif