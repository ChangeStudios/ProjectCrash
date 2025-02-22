/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamApps
*/

#include "SteamApps/SteamAppsAsyncActions.h"
#include "SteamApps/SteamAppsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProAppsAsyncActionGetFileDetails
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProAppsAsyncActionGetFileDetails* USteamCoreProAppsAsyncActionGetFileDetails::GetFileDetailsAsync(UObject* WorldContextObject, FString FileName, float Timeout)
{
	LogSteamCoreVerbose("");
#if WITH_STEAMCORE

	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProAppsAsyncActionGetFileDetails>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProAppsGetFileDetails(AsyncObject, FileName, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;	
	}
#endif
	
	return nullptr;
}

void USteamCoreProAppsAsyncActionGetFileDetails::HandleCallback(const FFileDetailsResult& Data, bool bWasSuccessful)
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
