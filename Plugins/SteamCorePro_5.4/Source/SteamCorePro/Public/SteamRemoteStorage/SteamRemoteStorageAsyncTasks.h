/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamRemoteStorage
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamRemoteStorageTypes.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnFileWriteAsync m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync(const FOnFileWriteAsync Callback, const FString File, const TArray<uint8> Data, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_File(File)
		  , m_Data(Data)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync() = delete;
protected:
	RemoteStorageFileWriteAsyncComplete_t m_CallbackResults;
	FString m_File;
	TArray<uint8> m_Data;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRemoteStorageFileWriteAsync")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnFileReadAsync m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync(const FOnFileReadAsync Callback, const FString File, const int32 Offset, const int32 BytesToRead, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_File(File)
		  , m_Offset(Offset)
		  , m_BytesToRead(BytesToRead)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync() = delete;
protected:
	RemoteStorageFileReadAsyncComplete_t m_CallbackResults;
	FString m_File;
	int32 m_Offset;
	int32 m_BytesToRead;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRemoteStorageFileReadAsync")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnFileShareAsync m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare(const FOnFileShareAsync Callback, const FString File, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_File(File)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare() = delete;
protected:
	RemoteStorageFileShareResult_t m_CallbackResults;
	FString m_File;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRemoteStorageFileShare")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnUGCDownloadAsync m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload(const FOnUGCDownloadAsync Callback, const FSteamUGCHandle Content, const int32 Priority, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Content(Content)
		  , m_Priority(Priority)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload() = delete;
protected:
	RemoteStorageDownloadUGCResult_t m_CallbackResults;
	FSteamUGCHandle m_Content;
	int32 m_Priority;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownload")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnUGCDownloadToLocationAsync m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation(const FOnUGCDownloadToLocationAsync Callback, const FSteamUGCHandle Content, const FString Location, const int32 Priority, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Content(Content)
		  , m_Location(Location)
		  , m_Priority(Priority)
	{
	}

private:
	FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation() = delete;
protected:
	RemoteStorageDownloadUGCResult_t m_CallbackResults;
	FSteamUGCHandle m_Content;
	FString m_Location;
	int32 m_Priority;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProRemoteStorageUGCDownloadToLocation")); }
};
#endif