/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmaking
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamPartiesTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProPartiesJoinParty
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProPartiesJoinParty : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnJoinParty m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProPartiesJoinParty(const FOnJoinParty Callback, const FPartyBeaconID BeaconId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_BeaconID(BeaconId)
	{
	}

	FOnlineAsyncTaskSteamCoreProPartiesJoinParty(USteamCoreProAsyncAction* AsyncObject, const FPartyBeaconID BeaconId, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_BeaconID(BeaconId)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProPartiesJoinParty() = delete;
protected:
	JoinPartyCallback_t m_CallbackResults;
	FPartyBeaconID m_BeaconID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProPartiesJoinParty")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnCreateBeacon m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon(const FOnCreateBeacon Callback, const int32 OpenSlots, const FSteamPartyBeaconLocation BeaconLocation, const FString ConnectString, const FString Metadata, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_OpenSlots(OpenSlots)
		  , m_BeaconLocation(BeaconLocation)
		  , m_ConnectString(ConnectString)
		  , m_Metadata(Metadata)
	{
	}

	FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon(USteamCoreProAsyncAction* AsyncObject, const int32 OpenSlots, const FSteamPartyBeaconLocation BeaconLocation, const FString ConnectString, const FString Metadata, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_OpenSlots(OpenSlots)
		  , m_BeaconLocation(BeaconLocation)
		  , m_ConnectString(ConnectString)
		  , m_Metadata(Metadata)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon() = delete;
protected:
	CreateBeaconCallback_t m_CallbackResults;
	int32 m_OpenSlots;
	FSteamPartyBeaconLocation m_BeaconLocation;
	FString m_ConnectString;
	FString m_Metadata;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnChangeNumOpenSlots m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots(const FOnChangeNumOpenSlots Callback, const FPartyBeaconID Beacon, const int32 OpenSlots, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Beacon(Beacon)
		  , m_OpenSlots(OpenSlots)
	{
	}

	FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots(USteamCoreProAsyncAction* AsyncObject, const FPartyBeaconID Beacon, const int32 OpenSlots, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Beacon(Beacon)
		  , m_OpenSlots(OpenSlots)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots() = delete;
protected:
	ChangeNumOpenSlotsCallback_t m_CallbackResults;
	FPartyBeaconID m_Beacon;
	int32 m_OpenSlots;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots")); }
};
#endif