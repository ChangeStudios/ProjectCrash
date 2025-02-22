/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUGC
*/

#include "SteamUGC/SteamUGCAsyncActions.h"
#include "SteamUGC/SteamUGCAsyncTasks.h"
#include "SteamUGC/SteamUGC.h"
#include "SteamCorePro/SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		USteamCoreProUGCAsyncActionStopPlaytimeTrackingForAllItems
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionStopPlaytimeTrackingForAllItems* USteamCoreProUGCAsyncActionStopPlaytimeTrackingForAllItems::StopPlaytimeTrackingForAllItemsAsync(UObject* WorldContextObject, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionStopPlaytimeTrackingForAllItems>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTrackingForAllItems(AsyncObject, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionStopPlaytimeTrackingForAllItems::HandleCallback(const FStopPlaytimeTrackingResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionStopPlaytimeTracking
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionStopPlaytimeTracking* USteamCoreProUGCAsyncActionStopPlaytimeTracking::StopPlaytimeTrackingAsync(UObject* WorldContextObject, TArray<FPublishedFileID> publishedFileIDs, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionStopPlaytimeTracking>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCStopPlaytimeTracking(AsyncObject, publishedFileIDs, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionStopPlaytimeTracking::HandleCallback(const FStopPlaytimeTrackingResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionStartPlaytimeTracking
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionStartPlaytimeTracking* USteamCoreProUGCAsyncActionStartPlaytimeTracking::StartPlaytimeTrackingAsync(UObject* WorldContextObject, TArray<FPublishedFileID> publishedFileIDs, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionStartPlaytimeTracking>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCStartPlaytimeTracking(AsyncObject, publishedFileIDs, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionStartPlaytimeTracking::HandleCallback(const FStartPlaytimeTrackingResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionUnsubscribeItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionUnsubscribeItem* USteamCoreProUGCAsyncActionUnsubscribeItem::UnsubscribeItemAsync(UObject* WorldContextObject, FPublishedFileID publishedFileIDs, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionUnsubscribeItem>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCUnsubscribeItem(AsyncObject, publishedFileIDs, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionUnsubscribeItem::HandleCallback(const FRemoteStorageSubscribePublishedFileResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionSubscribeItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionSubscribeItem* USteamCoreProUGCAsyncActionSubscribeItem::SubscribeItemAsync(UObject* WorldContextObject, FPublishedFileID publishedFileIDs, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionSubscribeItem>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCSubscribeItem(AsyncObject, publishedFileIDs, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionSubscribeItem::HandleCallback(const FRemoteStorageSubscribePublishedFileResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionRemoveItemFromFavorites
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionRemoveItemFromFavorites* USteamCoreProUGCAsyncActionRemoveItemFromFavorites::RemoveItemFromFavoritesAsync(UObject* WorldContextObject, int32 appID, FPublishedFileID publishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionRemoveItemFromFavorites>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCRemoveItemFromFavorites(AsyncObject, appID, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionRemoveItemFromFavorites::HandleCallback(const FUserFavoriteItemsListChanged& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionAddItemToFavorites
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionAddItemToFavorites* USteamCoreProUGCAsyncActionAddItemToFavorites::AddItemToFavoritesAsync(UObject* WorldContextObject, int32 appID, FPublishedFileID publishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionAddItemToFavorites>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCAddItemToFavorites(AsyncObject, appID, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionAddItemToFavorites::HandleCallback(const FUserFavoriteItemsListChanged& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionGetUserItemVote
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionGetUserItemVote* USteamCoreProUGCAsyncActionGetUserItemVote::GetUserItemVoteAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionGetUserItemVote>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCGetUserItemVote(AsyncObject, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionGetUserItemVote::HandleCallback(const FGetUserItemVoteResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionSetUserItemVote
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionSetUserItemVote* USteamCoreProUGCAsyncActionSetUserItemVote::SetUserItemVoteAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, bool bVoteUp, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionSetUserItemVote>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCSetUserItemVote(AsyncObject, bVoteUp, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionSetUserItemVote::HandleCallback(const FSetUserItemVoteResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionSubmitItemUpdate
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionSubmitItemUpdate* USteamCoreProUGCAsyncActionSubmitItemUpdate::SubmitItemUpdateAsync(UObject* WorldContextObject, FUGCUpdateHandle Handle, FString ChangeNote, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionSubmitItemUpdate>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCSubmitItemUpdate(AsyncObject, Handle, ChangeNote, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionSubmitItemUpdate::HandleCallback(const FSubmitItemUpdateResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionCreateItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionCreateItem* USteamCoreProUGCAsyncActionCreateItem::CreateItemAsync(UObject* WorldContextObject, int32 ConsumerAppID, ESteamWorkshopFileType FileType, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionCreateItem>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCCreateItem(AsyncObject, ConsumerAppID, FileType, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionCreateItem::HandleCallback(const FCreateItemResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionSendQueryUGCRequest
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionSendQueryUGCRequest* USteamCoreProUGCAsyncActionSendQueryUGCRequest::SendQueryUGCRequestAsync(UObject* WorldContextObject, FUGCQueryHandle Handle, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionSendQueryUGCRequest>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCSendQueryUGCRequest(AsyncObject, Handle, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionSendQueryUGCRequest::HandleCallback(const FSteamUGCQueryCompleted& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionAddAppDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionAddAppDependency* USteamCoreProUGCAsyncActionAddAppDependency::AddAppDependencyAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, int32 appID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionAddAppDependency>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCAddAppDependency(AsyncObject, publishedFileID, appID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionAddAppDependency::HandleCallback(const FAddAppDependencyResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionRemoveAppDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionRemoveAppDependency* USteamCoreProUGCAsyncActionRemoveAppDependency::RemoveAppDependencyAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, int32 appID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionRemoveAppDependency>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCRemoveAppDependency(AsyncObject, publishedFileID, appID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionRemoveAppDependency::HandleCallback(const FRemoveAppDependencyResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionAddUGCDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionAddUGCDependency* USteamCoreProUGCAsyncActionAddUGCDependency::AddDependencyAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, FPublishedFileID ChildPublishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionAddUGCDependency>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCAddUGCDependency(AsyncObject, publishedFileID, ChildPublishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionAddUGCDependency::HandleCallback(const FAddUGCDependencyResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionRemoveUGCDependency
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionRemoveUGCDependency* USteamCoreProUGCAsyncActionRemoveUGCDependency::RemoveDependencyAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, FPublishedFileID ChildPublishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionRemoveUGCDependency>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCRemoveUGCDependency(AsyncObject, publishedFileID, ChildPublishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionRemoveUGCDependency::HandleCallback(const FRemoveUGCDependencyResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionDeleteItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionDeleteItem* USteamCoreProUGCAsyncActionDeleteItem::DeleteItemAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionDeleteItem>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCDeleteItem(AsyncObject, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionDeleteItem::HandleCallback(const FUGCDeleteItemResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionGetAppDependencies
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionGetAppDependencies* USteamCoreProUGCAsyncActionGetAppDependencies::GetAppDependenciesAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionGetAppDependencies>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCGetAppDependencies(AsyncObject, publishedFileID, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionGetAppDependencies::HandleCallback(const FGetAppDependenciesResult& Data, bool bWasSuccessful)
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
//		USteamCoreProUGCAsyncActionDownloadItem
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
USteamCoreProUGCAsyncActionDownloadItem* USteamCoreProUGCAsyncActionDownloadItem::DownloadItemAsync(UObject* WorldContextObject, FPublishedFileID publishedFileID, bool bHighPriority, float Timeout)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (USteamCoreProSubsystem* Subsystem = USteamCoreProSubsystem::Get())
	{
		const auto AsyncObject = NewObject<USteamCoreProUGCAsyncActionDownloadItem>();
		const auto Task = new FOnlineAsyncTaskSteamCoreProUGCDownloadItem(AsyncObject, publishedFileID, bHighPriority, Timeout);
		AsyncObject->RegisterWithGameInstance(WorldContextObject);
		
		Subsystem->QueueAsyncTask(Task);
		AsyncObject->Activate();

		return AsyncObject;
	}
#endif

	return nullptr;
}

void USteamCoreProUGCAsyncActionDownloadItem::HandleCallback(const FDownloadItemResult& Data, bool bWasSuccessful)
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
