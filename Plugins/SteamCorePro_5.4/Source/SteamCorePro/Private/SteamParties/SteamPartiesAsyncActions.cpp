/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/isteamparties
*/

#include "SteamParties/SteamPartiesAsyncActions.h"
#include "SteamParties/SteamPartiesAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProSteamPartiesAsyncActionCreateLobby
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProSteamPartiesAsyncActionJoinParty* USteamCoreProSteamPartiesAsyncActionJoinParty::JoinPartyAsync(UObject* WorldContextObject, FPartyBeaconID BeaconId, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProSteamPartiesAsyncActionJoinParty>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProPartiesJoinParty(AsyncObject, BeaconId, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif
	
	return nullptr;
}

void USteamCoreProSteamPartiesAsyncActionJoinParty::HandleCallback(const FJoinPartyData& Data, bool bWasSuccessful)
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

USteamCoreProSteamPartiesAsyncActionCreateBeacon* USteamCoreProSteamPartiesAsyncActionCreateBeacon::CreateBeaconAsync(UObject* WorldContextObject, int32 OpenSlots, FSteamPartyBeaconLocation BeaconLocation, FString ConnectString, FString Metadata, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProSteamPartiesAsyncActionCreateBeacon>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProPartiesCreateBeacon(AsyncObject, OpenSlots, BeaconLocation, ConnectString, Metadata, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProSteamPartiesAsyncActionCreateBeacon::HandleCallback(const FCreateBeaconData& Data, bool bWasSuccessful)
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

USteamCoreProSteamPartiesAsyncActionChangeNumOpenSlots* USteamCoreProSteamPartiesAsyncActionChangeNumOpenSlots::ChangeNumOpenSlotsAsync(UObject* WorldContextObject, FPartyBeaconID BeaconId, int32 OpenSlots, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProSteamPartiesAsyncActionChangeNumOpenSlots>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProPartiesChangeNumOpenSlots(AsyncObject, BeaconId, OpenSlots, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProSteamPartiesAsyncActionChangeNumOpenSlots::HandleCallback(const FChangeNumOpenSlotsData& Data, bool bWasSuccessful)
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
