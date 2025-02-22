/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmaking
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamMatchmakingTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnCreateLobby m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby(FOnCreateLobby Callback, const ELobbyType LobbyType, const int32 MaxLobbyMembers, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_LobbyType(LobbyType)
		  , m_MaxLobbyMembers(MaxLobbyMembers)
	{
	}

	FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby(USteamCoreProAsyncAction* AsyncObject, const ELobbyType LobbyType, const int32 MaxLobbyMembers, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_LobbyType(LobbyType)
		  , m_MaxLobbyMembers(MaxLobbyMembers)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby() = delete;
protected:
	LobbyCreated_t m_CallbackResults;
	ELobbyType m_LobbyType;
	int32 m_MaxLobbyMembers;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRequestLobbyList
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRequestLobbyList : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestLobbyList m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRequestLobbyList(FOnRequestLobbyList Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProRequestLobbyList(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProRequestLobbyList() = delete;
protected:
	LobbyMatchList_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRequestLobbyList")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProJoinLobby
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProJoinLobby : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnJoinLobby m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProJoinLobby(class FOnJoinLobby Callback, FSteamID SteamIDLobby, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDLobby(SteamIDLobby)
	{
	}

	FOnlineAsyncTaskSteamCoreProJoinLobby(USteamCoreProAsyncAction* AsyncObject, FSteamID SteamIDLobby, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamIDLobby(SteamIDLobby)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProJoinLobby() = delete;
protected:
	LobbyEnter_t m_CallbackResults;
	FSteamID m_SteamIDLobby;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProJoinLobby")); }
};
#endif