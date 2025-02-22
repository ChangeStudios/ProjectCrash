/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCoreTypes.h"

#if WITH_STEAMCORE

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreReadSharedFile : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreReadSharedFile()
		: m_CallbackResults(),
		  m_SharedHandle(k_UGCHandleInvalid)
		  , m_bInit(false)
	{
	}

PACKAGE_SCOPE:
	RemoteStorageDownloadUGCResult_t m_CallbackResults;

public:
	FOnlineAsyncTaskSteamCoreReadSharedFile(class FOnlineSubsystemSteamCore* InSubsystem, const FSharedContentHandleSteam& InSharedHandle)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_CallbackResults(),
		  m_SharedHandle(InSharedHandle)
		  , m_bInit(false)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

private:
	FSharedContentHandleSteam m_SharedHandle;
	bool m_bInit;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreEnumerateUserFiles : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreEnumerateUserFiles() = delete;

public:
	FOnlineAsyncTaskSteamCoreEnumerateUserFiles(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_UserId(InUserId.AsShared())
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

private:
	FUniqueNetIdSteamRef m_UserId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreReadUserFile : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreReadUserFile() = delete;

public:
	FOnlineAsyncTaskSteamCoreReadUserFile(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, const FString& InFileName)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_UserId(InUserId.AsShared()),
		  m_FileName(InFileName)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

PACKAGE_SCOPE:
	FUniqueNetIdSteamRef m_UserId;
	FString m_FileName;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreWriteUserFile : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreWriteUserFile() = delete;

PACKAGE_SCOPE:
	bool WriteUserFile(const FUniqueNetId& UserId, const FString& FileToWrite, const TArray<uint8>& Contents);

public:
	FOnlineAsyncTaskSteamCoreWriteUserFile(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, const FString& InFileName, const TArray<uint8>& InContents)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_Contents(InContents),
		  m_UserId(InUserId.AsShared()),
		  m_FileName(InFileName)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

PACKAGE_SCOPE:
	TArray<uint8> m_Contents;
	FUniqueNetIdSteamRef m_UserId;
	FString m_FileName;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreDeleteUserFile : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreDeleteUserFile() = delete;

public:
	FOnlineAsyncTaskSteamCoreDeleteUserFile(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, const FString& InFileName, bool bInShouldCloudDelete, bool bInShouldLocallyDelete)
		: FOnlineAsyncTaskSteamCore(InSubsystem, k_uAPICallInvalid),
		  m_bShouldCloudDelete(bInShouldCloudDelete),
		  m_bShouldLocallyDelete(bInShouldLocallyDelete),
		  m_UserId(InUserId.AsShared()),
		  m_FileName(InFileName)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bShouldCloudDelete;
	bool m_bShouldLocallyDelete;
	FUniqueNetIdSteamRef m_UserId;
	FString m_FileName;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreWriteSharedFile : public FOnlineAsyncTaskSteamCoreWriteUserFile
{
	FOnlineAsyncTaskSteamCoreWriteSharedFile() = delete;

PACKAGE_SCOPE:
	RemoteStorageFileShareResult_t m_CallbackResults;

public:
	FOnlineAsyncTaskSteamCoreWriteSharedFile(class FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, const FString& InFileName, const TArray<uint8>& InContents)
		: FOnlineAsyncTaskSteamCoreWriteUserFile(InSubsystem, InUserId, InFileName, InContents),
		  m_CallbackResults(), m_bInit(false)
	{
	}

	virtual FString ToString() const override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bInit;
};
#endif