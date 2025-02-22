/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "Interfaces/OnlineSharedCloudInterface.h"
#include "OnlineSubsystemSteamCorePackage.h"

struct FCloudFileSteam;

#if WITH_STEAMCORE

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineSharedCloudSteamCore : public IOnlineSharedCloud
{
	FOnlineSharedCloudSteamCore()
		: m_SteamSubsystem(nullptr)
	{
	}

PACKAGE_SCOPE:
	FOnlineSharedCloudSteamCore(class FOnlineSubsystemSteamCore* InSubsystem)
		: m_SteamSubsystem(InSubsystem)
	{
	}

	FCloudFileSteam* GetSharedCloudFile(const FSharedContentHandle& SharedHandle);

public:
	virtual ~FOnlineSharedCloudSteamCore() override
	{
		ClearSharedFiles();
	}

	// IOnlineSharedCloud
	virtual bool GetSharedFileContents(const FSharedContentHandle& SharedHandle, TArray<uint8>& FileContents) override;
	virtual bool ClearSharedFiles() override final;
	virtual bool ClearSharedFile(const FSharedContentHandle& SharedHandle) override;
	virtual bool ReadSharedFile(const FSharedContentHandle& SharedHandle) override;
	virtual bool WriteSharedFile(const FUniqueNetId& UserId, const FString& Filename, TArray<uint8>& FileContents) override;
	virtual void GetDummySharedHandlesForTest(TArray<TSharedRef<FSharedContentHandle>>& OutHandles) override;

protected:
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	TArray<FCloudFileSteam*> m_SharedFileCache;
};

typedef TSharedPtr<FOnlineSharedCloudSteamCore, ESPMode::ThreadSafe> FOnlineSharedCloudSteamCorePtr;
#endif