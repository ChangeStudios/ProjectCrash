/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamGameServerStats
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamGameServerStatsTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnServerRequestUserStats m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS(const FOnServerRequestUserStats Callback, const FSteamID SteamIDUser, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDUser(SteamIDUser)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS() = delete;
protected:
	GSStatsReceived_t m_CallbackResults;
	FSteamID m_SteamIDUser;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProGameServerStatsUserStatsGS")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnServerStoreUserStats m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats(const FOnServerStoreUserStats Callback, const FSteamID SteamIDUser, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDUser(SteamIDUser)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats() = delete;
protected:
	GSStatsStored_t m_CallbackResults;
	FSteamID m_SteamIDUser;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProGameServerStatsStoreUserStats")); }
};

#endif