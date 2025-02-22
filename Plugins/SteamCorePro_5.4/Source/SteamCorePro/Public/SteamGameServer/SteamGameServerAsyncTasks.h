/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamGameServer
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamGameServerTypes.h"

#if WITH_STEAMCORE

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnAssociateWithClan m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan(const FOnAssociateWithClan Callback, const FSteamID SteamIDClan, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamIDClan(SteamIDClan)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan() = delete;
protected:
	AssociateWithClanResult_t m_CallbackResults;
	FSteamID m_SteamIDClan;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnComputeNewPlayerCompatibility m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility(const FOnComputeNewPlayerCompatibility Callback, const FSteamID SteamIDNewPlayer, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_SteamIDNewPlayer(SteamIDNewPlayer)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility() = delete;
protected:
	ComputeNewPlayerCompatibilityResult_t m_CallbackResults;
	FSteamID m_SteamIDNewPlayer;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility")); }
};
#endif