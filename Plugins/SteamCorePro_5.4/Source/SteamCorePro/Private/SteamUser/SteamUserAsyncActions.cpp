/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUGC
*/

#include "SteamUser/SteamUserAsyncActions.h"
#include "SteamUser/SteamUserAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserAsyncActionRequestEncryptedAppTicket
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUserAsyncActionRequestEncryptedAppTicket* USteamCoreProUserAsyncActionRequestEncryptedAppTicket::RequestEncryptedAppTicketAsync(UObject* WorldContextObject, TArray<uint8> DataToInclude, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUserAsyncActionRequestEncryptedAppTicket>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket(AsyncObject, DataToInclude, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUserAsyncActionRequestEncryptedAppTicket::HandleCallback(const FEncryptedAppTicketResponse& Data, bool bWasSuccessful)
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUserAsyncActionRequestStoreAuthURL
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUserAsyncActionRequestStoreAuthURL* USteamCoreProUserAsyncActionRequestStoreAuthURL::RequestStoreAuthURLAsync(UObject* WorldContextObject, FString RedirectURL, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUserAsyncActionRequestStoreAuthURL>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL(AsyncObject, RedirectURL, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUserAsyncActionRequestStoreAuthURL::HandleCallback(const FStoreAuthURLResponse& Data, bool bWasSuccessful)
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
