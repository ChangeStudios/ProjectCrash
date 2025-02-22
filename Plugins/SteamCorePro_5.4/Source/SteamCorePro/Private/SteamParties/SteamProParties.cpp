/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/isteamparties
*/

#include "SteamParties/SteamProParties.h"
#include "SteamParties/SteamPartiesAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProParties::USteamProParties()
{
#if WITH_STEAMCORE
	if (SteamParties())
	{
		OnJoinPartyCallback.Register(this, &USteamProParties::OnJoinPartyUpdated);
		OnCreateBeaconCallback.Register(this, &USteamProParties::OnCreateBeaconUpdated);
		OnReservationNotificationCallback.Register(this, &USteamProParties::OnReservationNotificationUpdated);
		OnChangeNumOpenSlotsCallback.Register(this, &USteamProParties::OnChangeNumOpenSlotsUpdated);
		OnAvailableBeaconLocationsCallback.Register(this, &USteamProParties::OnAvailableBeaconLocationsUpdated);
		OnActiveBeaconsCallback.Register(this, &USteamProParties::OnActiveBeaconsUpdated);

		if (IsRunningDedicatedServer())
		{
			OnJoinPartyCallback.SetGameserverFlag();
			OnCreateBeaconCallback.SetGameserverFlag();
			OnReservationNotificationCallback.SetGameserverFlag();
			OnChangeNumOpenSlotsCallback.SetGameserverFlag();
			OnAvailableBeaconLocationsCallback.SetGameserverFlag();
			OnActiveBeaconsCallback.SetGameserverFlag();
		}
	}
#endif
}

USteamProParties::~USteamProParties()
{
#if WITH_STEAMCORE
	if (SteamParties())
	{
		OnJoinPartyCallback.Unregister();
		OnCreateBeaconCallback.Unregister();
		OnChangeNumOpenSlotsCallback.Unregister();
		OnAvailableBeaconLocationsCallback.Unregister();
		OnReservationNotificationCallback.Unregister();
		OnActiveBeaconsCallback.Unregister();
	}
#endif
}

USteamProParties* USteamProParties::GetSteamParties()
{
#if WITH_STEAMCORE
	return SteamParties() ? ThisClass::StaticClass()->GetDefaultObject<USteamProParties>() : nullptr;
#else
	return nullptr;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


int32 USteamProParties::GetNumActiveBeacons()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		Result = SteamParties()->GetNumActiveBeacons();
	}
#endif

	return Result;
}

FPartyBeaconID USteamProParties::GetBeaconByIndex(int32 Index)
{
	LogSteamCoreVeryVerbose("");

	FPartyBeaconID Result;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		Result = SteamParties()->GetBeaconByIndex(Index);
	}
#endif

	return Result;
}

bool USteamProParties::GetBeaconDetails(FPartyBeaconID BeaconId, FSteamID& SteamIDBeaconOwner, FSteamPartyBeaconLocation& Location, FString& OutMetadata)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		CSteamID Owner = {};
		SteamPartyBeaconLocation_t SteamPartyBeaconLocation = {};

		TArray<char> MetaData;
		MetaData.SetNum(4096);

		bResult = SteamParties()->GetBeaconDetails(BeaconId, &Owner, &SteamPartyBeaconLocation, MetaData.GetData(), MetaData.Num());

		if (bResult)
		{
			SteamIDBeaconOwner = Owner;
			Location = SteamPartyBeaconLocation;
			OutMetadata = UTF8_TO_TCHAR(MetaData.GetData());
		}
	}
#endif

	return bResult;
}

void USteamProParties::JoinParty(const FOnJoinParty& Callback, FPartyBeaconID BeaconId)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamParties())
	{
		FOnlineAsyncTaskSteamCoreProPartiesJoinParty* Task = new FOnlineAsyncTaskSteamCoreProPartiesJoinParty(Callback, BeaconId);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProParties::GetNumAvailableBeaconLocations(int32& PuNumLocations)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		uint32 NumLocations;
		bResult = SteamParties()->GetNumAvailableBeaconLocations(&NumLocations);
		PuNumLocations = NumLocations;
	}
#endif

	return bResult;
}

bool USteamProParties::GetAvailableBeaconLocations(TArray<FSteamPartyBeaconLocation>& pLocationList, int32 uMaxNumLocations /*= 10*/)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		TArray<SteamPartyBeaconLocation_t> LocationList;
		LocationList.SetNum(uMaxNumLocations);

		bResult = SteamParties()->GetAvailableBeaconLocations(LocationList.GetData(), LocationList.Num());

		if (bResult)
		{
			for (auto& Element : LocationList)
			{
				pLocationList.Add(Element);
			}
		}
	}
#endif

	return bResult;
}


void USteamProParties::CreateBeacon(const FOnCreateBeacon& Callback, int32 OpenSlots, FSteamPartyBeaconLocation BeaconLocation, FString ConnectString, FString Metadata)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamParties())
	{
		FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon* Task = new FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon(Callback, OpenSlots, BeaconLocation, ConnectString, Metadata);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProParties::OnReservationCompleted(FPartyBeaconID BeaconId, FSteamID SteamIDUser)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamParties())
	{
		SteamParties()->OnReservationCompleted(BeaconId, SteamIDUser);
	}
#endif
}

void USteamProParties::CancelReservation(FPartyBeaconID BeaconId, FSteamID SteamIDUser)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamParties())
	{
		SteamParties()->CancelReservation(BeaconId, SteamIDUser);
	}
#endif
}


void USteamProParties::ChangeNumOpenSlots(const FOnChangeNumOpenSlots& Callback, FPartyBeaconID BeaconId, int32 OpenSlots)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamParties())
	{
		FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots* Task = new FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots(Callback, BeaconId, OpenSlots);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProParties::DestroyBeacon(FPartyBeaconID BeaconId)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		bResult = SteamParties()->DestroyBeacon(BeaconId);
	}
#endif

	return bResult;
}

bool USteamProParties::GetBeaconLocationData(FSteamPartyBeaconLocation BeaconLocation, ESteamPartiesBeaconLocationData EData, FString& PCHDataStringOut)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParties())
	{
		TArray<char> OutString;
		OutString.SetNum(4096);

		SteamPartyBeaconLocation_t SteamBeaconLocation;
		SteamBeaconLocation.m_eType = static_cast<ESteamPartyBeaconLocationType>(BeaconLocation.Type);
		SteamBeaconLocation.m_ulLocationID = FCString::Atoi64(*BeaconLocation.LocationId);

		bResult = SteamParties()->GetBeaconLocationData(SteamBeaconLocation, static_cast<ESteamPartyBeaconLocationData>(EData), OutString.GetData(), OutString.Num());

		if (bResult)
		{
			PCHDataStringOut = UTF8_TO_TCHAR(OutString.GetData());
		}
	}
#endif

	return bResult;
}

#if WITH_STEAMCORE
void USteamProParties::OnActiveBeaconsUpdated(ActiveBeaconsUpdated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ActiveBeaconsDelegate.Broadcast();
	});
}

void USteamProParties::OnAvailableBeaconLocationsUpdated(AvailableBeaconLocationsUpdated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		AvailableBeaconLocationsDelegate.Broadcast();
	});
}

void USteamProParties::OnReservationNotificationUpdated(ReservationNotificationCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ReservationNotificationDelegate.Broadcast(Data);
	});
}

void USteamProParties::OnJoinPartyUpdated(JoinPartyCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		JoinPartyDelegate.Broadcast(Data);
	});
}

void USteamProParties::OnCreateBeaconUpdated(CreateBeaconCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		CreateBeaconDelegate.Broadcast(Data);
	});
}

void USteamProParties::OnChangeNumOpenSlotsUpdated(ChangeNumOpenSlotsCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ChangeNumOpenSlotsDelegate.Broadcast(Data);
	});
}
#endif
