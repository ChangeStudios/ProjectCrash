/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineUserCloudSteamCore : public IOnlineUserCloud
{
	FOnlineUserCloudSteamCore()
		: m_SteamSubsystem(nullptr)
	{
	}

PACKAGE_SCOPE:
	FOnlineUserCloudSteamCore(class FOnlineSubsystemSteamCore* InSubsystem)
		: m_SteamSubsystem(InSubsystem)
	{
	}

public:
	virtual ~FOnlineUserCloudSteamCore() override;

	// IOnlineUserCloud
	virtual bool GetFileContents(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents) override;
	virtual bool ClearFiles(const FUniqueNetId& UserId) override;
	virtual bool ClearFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual void EnumerateUserFiles(const FUniqueNetId& UserId) override;
	virtual void GetUserFileList(const FUniqueNetId& UserId, TArray<FCloudFileHeader>& UserFiles) override;
	virtual bool ReadUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool WriteUserFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents, bool bCompressBeforeUpload = false) override;
	virtual void CancelWriteUserFile(const FUniqueNetId& UserId, const FString& FileName) override;
	virtual bool DeleteUserFile(const FUniqueNetId& UserId, const FString& FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete) override;
	virtual bool RequestUsageInfo(const FUniqueNetId& UserId) override;
	virtual void DumpCloudState(const FUniqueNetId& UserId) override;
	virtual void DumpCloudFileState(const FUniqueNetId& UserId, const FString& FileName) override;

private:
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
};

typedef TSharedPtr<FOnlineUserCloudSteamCore, ESPMode::ThreadSafe> FOnlineUserCloudSteamCorePtr;
#endif