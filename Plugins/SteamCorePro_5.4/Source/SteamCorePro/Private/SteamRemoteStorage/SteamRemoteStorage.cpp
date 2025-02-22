/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamRemoteStorage
*/

#include "SteamRemoteStorage/SteamRemoteStorage.h"
#include "SteamRemoteStorage/SteamRemoteStorageAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProRemoteStorage::USteamProRemoteStorage()
{
#if WITH_STEAMCORE
	OnRemoteStorageUnsubscribePublishedFileResultCallback.Register(this, &USteamProRemoteStorage::OnRemoteStorageUnsubscribePublishedFileResult);
	OnRemoteStorageSubscribePublishedFileResultCallback.Register(this, &USteamProRemoteStorage::OnRemoteStorageSubscribePublishedFileResult);

	if (IsRunningDedicatedServer())
	{
		OnRemoteStorageSubscribePublishedFileResultCallback.SetGameserverFlag();
		OnRemoteStorageUnsubscribePublishedFileResultCallback.SetGameserverFlag();
	}
#endif
}

USteamProRemoteStorage::~USteamProRemoteStorage()
{
#if WITH_STEAMCORE
	OnRemoteStorageUnsubscribePublishedFileResultCallback.Unregister();
	OnRemoteStorageSubscribePublishedFileResultCallback.Unregister();
#endif
}

USteamProRemoteStorage* USteamProRemoteStorage::GetSteamRemoteStorage()
{
#if WITH_STEAMCORE
	return SteamRemoteStorage() ? ThisClass::StaticClass()->GetDefaultObject<USteamProRemoteStorage>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


bool USteamProRemoteStorage::FileDelete(FString File)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileDelete(TCHAR_TO_UTF8(*File));
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::FileExists(FString File)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileExists(TCHAR_TO_UTF8(*File));
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::FileForget(FString File)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileForget(TCHAR_TO_UTF8(*File));
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::FilePersisted(FString File)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FilePersisted(TCHAR_TO_UTF8(*File));
	}
#endif

	return bResult;
}

int32 USteamProRemoteStorage::FileRead(FString File, TArray<uint8>& OutBuffer, int32 DataToRead)
{
	LogSteamCoreVerbose("");

	int32 m_Result = 0;
	OutBuffer.Empty();

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		OutBuffer.SetNum(DataToRead);

		m_Result = SteamRemoteStorage()->FileRead(TCHAR_TO_UTF8(*File), OutBuffer.GetData(), OutBuffer.Num());
	}
#endif

	return m_Result;
}

void USteamProRemoteStorage::FileReadAsync(const FOnFileReadAsync& Callback, FString File, int32 Offset, int32 BytesToRead)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync* Task = new FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync(Callback, File, Offset, BytesToRead);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProRemoteStorage::FileReadAsyncComplete(FRemoteStorageFileReadAsyncComplete ReadCall, TArray<uint8>& OutBuffer, int32 BytesToRead)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

	OutBuffer.Empty();

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		OutBuffer.SetNum(BytesToRead);

		bResult = SteamRemoteStorage()->FileReadAsyncComplete(ReadCall.m_hFileReadAsync, OutBuffer.GetData(), OutBuffer.Num());
	}
#endif

	return bResult;
}

void USteamProRemoteStorage::FileShare(const FOnFileShareAsync& Callback, FString File)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare* Task = new FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare(Callback, File);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProRemoteStorage::FileWrite(FString File, TArray<uint8> Data)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileWrite(TCHAR_TO_UTF8(*File), Data.GetData(), Data.Num());
	}
#endif

	return bResult;
}

void USteamProRemoteStorage::FileWriteAsync(const FOnFileWriteAsync& Callback, FString File, TArray<uint8> Data)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync* Task = new FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync(Callback, File, Data);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProRemoteStorage::FileWriteStreamCancel(FUGCFileWriteStreamHandle Handle)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileWriteStreamCancel(Handle);
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::FileWriteStreamClose(FUGCFileWriteStreamHandle Handle)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileWriteStreamClose(Handle);
	}
#endif

	return bResult;
}

FUGCFileWriteStreamHandle USteamProRemoteStorage::FileWriteStreamOpen(FString File)
{
	LogSteamCoreVerbose("");

	FUGCFileWriteStreamHandle Result;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = FUGCFileWriteStreamHandle(SteamRemoteStorage()->FileWriteStreamOpen(TCHAR_TO_UTF8(*File)));
	}
#endif

	return Result;
}

bool USteamProRemoteStorage::FileWriteStreamWriteChunk(FUGCFileWriteStreamHandle Handle, TArray<uint8> Data)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->FileWriteStreamWriteChunk(Handle, Data.GetData(), Data.Num());
	}
#endif

	return bResult;
}

int32 USteamProRemoteStorage::GetCachedUGCCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->GetCachedUGCCount();
	}
#endif

	return Result;
}

FSteamUGCHandle USteamProRemoteStorage::GetCachedUGCHandle(int32 ICachedContent)
{
	LogSteamCoreVeryVerbose("");

	FSteamUGCHandle Result;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = FSteamUGCHandle(SteamRemoteStorage()->GetCachedUGCHandle(ICachedContent));
	}
#endif

	return Result;
}

int32 USteamProRemoteStorage::GetFileCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->GetFileCount();
	}
#endif

	return Result;
}

FString USteamProRemoteStorage::GetFileNameAndSize(int32 File, int32& OutFileSizeInBytes)
{
	LogSteamCoreVeryVerbose("");

	FString Result;
	OutFileSizeInBytes = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = UTF8_TO_TCHAR(SteamRemoteStorage()->GetFileNameAndSize(File, &OutFileSizeInBytes));
	}
#endif

	return Result;
}

int32 USteamProRemoteStorage::GetFileSize(FString File)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->GetFileSize(TCHAR_TO_UTF8(*File));
	}
#endif

	return Result;
}

int32 USteamProRemoteStorage::GetFileTimestamp(FString File)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->GetFileTimestamp(TCHAR_TO_UTF8(*File));
	}
#endif

	return Result;
}

bool USteamProRemoteStorage::GetQuota(int32& OutTotalBytes, int32& OutAvailableBytes)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	uint64 TotalBytes = 0;
	uint64 AvailableBytes = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->GetQuota((&TotalBytes), &AvailableBytes);
	}
#endif

	OutTotalBytes = TotalBytes;
	OutAvailableBytes = AvailableBytes;

	return bResult;
}

ESteamRemoteStoragePlatform USteamProRemoteStorage::GetSyncPlatforms(FString File)
{
	LogSteamCoreVeryVerbose("");

	ESteamRemoteStoragePlatform Result = ESteamRemoteStoragePlatform::None;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = static_cast<ESteamRemoteStoragePlatform>(SteamRemoteStorage()->GetSyncPlatforms(TCHAR_TO_UTF8(*File)));
	}
#endif

	return Result;
}

bool USteamProRemoteStorage::GetUGCDetails(FSteamUGCHandle Handle, int32& OutAppID, FString& OutName, int32& OutFileSizeInBytes, FSteamID& OutSteamIdOwner)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

	uint32 AppId = 0;
	OutName.Empty();
	OutFileSizeInBytes = 0;
	OutSteamIdOwner = FSteamID();

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		CSteamID SteamIdOwner;
		char* Name = new char[256];
		int32 FileSizeInBytes = 0;

		bResult = SteamRemoteStorage()->GetUGCDetails(Handle, &AppId, &Name, &FileSizeInBytes, &SteamIdOwner);

		if (bResult)
		{
			OutFileSizeInBytes = FileSizeInBytes;
			OutSteamIdOwner = SteamIdOwner;
			OutAppID = AppId;
			OutName = UTF8_TO_TCHAR(Name);
		}

		delete[] Name;
	}
#endif

	return bResult;
}

void USteamProRemoteStorage::UGCDownload(const FOnUGCDownloadAsync& Callback, FSteamUGCHandle Content, int32 Priority)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload* Task = new FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload(Callback, Content, Priority);
		QueueAsyncTask(Task);
	}
#endif
}

int32 USteamProRemoteStorage::UGCRead(FSteamUGCHandle Content, TArray<uint8>& OutData, int32 DataToRead, int32 Offset, ESteamUGCReadAction Action)
{
	LogSteamCoreVerbose("");

	int32 Result = 0;

	OutData.Empty();
	OutData.AddUninitialized(DataToRead);

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->UGCRead(Content, OutData.GetData(), OutData.Num(), Offset, static_cast<EUGCReadAction>(Action));
	}
#endif

	return Result;
}

void USteamProRemoteStorage::UGCDownloadToLocation(const FOnUGCDownloadToLocationAsync& Callback, FSteamUGCHandle Content, FString Location, int32 Priority)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation* Task = new FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation(Callback, Content, Location, Priority);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProRemoteStorage::GetUGCDownloadProgress(FSteamUGCHandle Handle, int32& OutBytesDownloaded, int32& OutBytesExpected)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

	OutBytesDownloaded = 0;
	OutBytesExpected = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->GetUGCDownloadProgress(Handle, &OutBytesDownloaded, &OutBytesExpected);
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::IsCloudEnabledForAccount()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->IsCloudEnabledForAccount();
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::IsCloudEnabledForApp()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->IsCloudEnabledForApp();
	}
#endif

	return bResult;
}

void USteamProRemoteStorage::SetCloudEnabledForApp(bool bEnabled)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		SteamRemoteStorage()->SetCloudEnabledForApp(bEnabled);
	}
#endif
}

bool USteamProRemoteStorage::SetSyncPlatforms(FString File, ESteamRemoteStoragePlatform RemoteStoragePlatform)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->SetSyncPlatforms(TCHAR_TO_UTF8(*File), static_cast<ERemoteStoragePlatform>(RemoteStoragePlatform));
	}
#endif

	return bResult;
}

int32 USteamProRemoteStorage::GetLocalFileChangeCount()
{
	LogSteamCoreVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		Result = SteamRemoteStorage()->GetLocalFileChangeCount();
	}
#endif

	return Result;
}

bool USteamProRemoteStorage::BeginFileWriteBatch()
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->BeginFileWriteBatch();
	}
#endif

	return bResult;
}

bool USteamProRemoteStorage::EndFileWriteBatch()
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamRemoteStorage())
	{
		bResult = SteamRemoteStorage()->EndFileWriteBatch();
	}
#endif

	return bResult;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProRemoteStorage::OnRemoteStorageUnsubscribePublishedFileResult(RemoteStorageUnsubscribePublishedFileResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RemoteStorageUnsubscribePublishedFileResult.Broadcast(Data);
	});
}

void USteamProRemoteStorage::OnRemoteStorageSubscribePublishedFileResult(RemoteStorageSubscribePublishedFileResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RemoteStorageSubscribePublishedFileResult.Broadcast(Data);
	});
}

void USteamProRemoteStorage::OnRemoteStoragePublishedFileUnsubscribed(RemoteStoragePublishedFileUnsubscribed_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RemoteStoragePublishedFileUnsubscribed.Broadcast(Data);
	});
}

void USteamProRemoteStorage::OnRemoteStoragePublishedFileSubscribed(RemoteStoragePublishedFileSubscribed_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		RemoteStoragePublishedFileSubscribed.Broadcast(Data);
	});
}
#endif
