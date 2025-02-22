/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUGC
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamUserTypes.h"

#if WITH_STEAMCORE

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestEncryptedAppTicket m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket(const FOnRequestEncryptedAppTicket Callback, const TArray<uint8> DataToInclude, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Data(DataToInclude)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket(USteamCoreProAsyncAction* AsyncObject, const TArray<uint8> DataToInclude, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Data(DataToInclude)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket() = delete;
protected:
	EncryptedAppTicketResponse_t m_CallbackResults;
	TArray<uint8> m_Data;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnStoreAuthURLResponse m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL(const FOnStoreAuthURLResponse Callback, const FString RedirectURL, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_RedirectURL(RedirectURL)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL(USteamCoreProAsyncAction* AsyncObject, const FString RedirectURL, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_RedirectURL(RedirectURL)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL() = delete;
protected:
	StoreAuthURLResponse_t m_CallbackResults;
	FString m_RedirectURL;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL")); }
};
#endif