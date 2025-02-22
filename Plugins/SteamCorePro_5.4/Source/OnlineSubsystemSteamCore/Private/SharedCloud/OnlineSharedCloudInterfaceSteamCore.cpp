/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SharedCloud/OnlineSharedCloudInterfaceSteamCore.h"
#include "SharedCloud/OnlineCloudTypesSteamCore.h"
#include "SharedCloud/OnlineSharedCloudAsyncTasksSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE

FCloudFileSteam* FOnlineSharedCloudSteamCore::GetSharedCloudFile(const FSharedContentHandle& SharedHandle)
{
	for (int32 FileIdx = 0; FileIdx < m_SharedFileCache.Num(); FileIdx++)
	{
		FCloudFileSteam* SharedFile = m_SharedFileCache[FileIdx];
		if (SharedFile->m_SharedHandle == *(FSharedContentHandleSteam*)&SharedHandle)
		{
			return SharedFile;
		}
	}

	// Always create a new one if it doesn't exist
	FCloudFileSteam* NewItem = new FCloudFileSteam(*(FSharedContentHandleSteam*)&SharedHandle);
	const int32 FileIdx = m_SharedFileCache.Add(NewItem);
	return m_SharedFileCache[FileIdx];
}

bool FOnlineSharedCloudSteamCore::GetSharedFileContents(const FSharedContentHandle& SharedHandle, TArray<uint8>& FileContents)
{
	const FCloudFileSteam* SharedFile = GetSharedCloudFile(SharedHandle);
	if (SharedFile && SharedFile->AsyncState == EOnlineAsyncTaskState::Done && SharedFile->Data.Num() > 0)
	{
		FileContents = SharedFile->Data;
		return true;
	}
	else
	{
		FileContents.Empty();
	}

	return false;
}

bool FOnlineSharedCloudSteamCore::ClearSharedFiles()
{
	for (int32 FileIdx = 0; FileIdx < m_SharedFileCache.Num(); FileIdx++)
	{
		const FCloudFileSteam* SharedFile = m_SharedFileCache[FileIdx];
		if (SharedFile->AsyncState == EOnlineAsyncTaskState::InProgress)
		{
			return false;
		}
	}

	for (int32 FileIdx = 0; FileIdx < m_SharedFileCache.Num(); FileIdx++)
	{
		const FCloudFileSteam* SharedFile = m_SharedFileCache[FileIdx];
		delete SharedFile;
	}

	m_SharedFileCache.Empty();
	return true;
}

bool FOnlineSharedCloudSteamCore::ClearSharedFile(const FSharedContentHandle& SharedHandle)
{
	for (int32 FileIdx = 0; FileIdx < m_SharedFileCache.Num(); FileIdx++)
	{
		const FCloudFileSteam* SharedFile = m_SharedFileCache[FileIdx];
		if (SharedFile->m_SharedHandle == *(FSharedContentHandleSteam*)&SharedHandle)
		{
			if (SharedFile->AsyncState != EOnlineAsyncTaskState::InProgress)
			{
				m_SharedFileCache.RemoveAtSwap(FileIdx);
				delete SharedFile;
				return true;
			}
			break;
		}
	}

	return false;
}

bool FOnlineSharedCloudSteamCore::ReadSharedFile(const FSharedContentHandle& SharedHandle)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;

	if (FCloudFileSteam* SharedFile = GetSharedCloudFile(SharedHandle))
	{
		SharedFile->AsyncState = EOnlineAsyncTaskState::InProgress;
		m_SteamSubsystem->QueueAsyncTask(new FOnlineAsyncTaskSteamCoreReadSharedFile(m_SteamSubsystem, *(FSharedContentHandleSteam*)&SharedHandle));
		bSuccess = true;
	}

	return bSuccess;
}

bool FOnlineSharedCloudSteamCore::WriteSharedFile(const FUniqueNetId& UserId, const FString& FileName, TArray<uint8>& FileContents)
{
	LogSteamCoreVerbose("");
	m_SteamSubsystem->QueueAsyncTask(new FOnlineAsyncTaskSteamCoreWriteSharedFile(m_SteamSubsystem, FUniqueNetIdSteam::Cast(UserId), FileName, FileContents));
	return true;
}

void FOnlineSharedCloudSteamCore::GetDummySharedHandlesForTest(TArray<TSharedRef<FSharedContentHandle>>& OutHandles)
{
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766135714)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766136144)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766136543)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766137039)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766137499)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766137928)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766138377)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766138784)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766139217)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766139630)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766140275)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766140713)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766141131)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766141899)));
	OutHandles.Add(MakeShareable(new FSharedContentHandleSteam(594715184766142348)));
}
#endif