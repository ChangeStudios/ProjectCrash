/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SharedCloud/OnlineSharedCloudAsyncTasksSteamCore.h"
#include "SharedCloud/OnlineCloudTypesSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "SharedCloud/OnlineSharedCloudInterfaceSteamCore.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE

FString FOnlineAsyncTaskSteamCoreReadSharedFile::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreReadSharedFile bWasSuccessful: %d Handle: %s"), WasSuccessful(), *m_SharedHandle.ToDebugString());
}

void FOnlineAsyncTaskSteamCoreReadSharedFile::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		if (SteamRemoteStorage() && m_SharedHandle.IsValid())
		{
			if (SteamUser()->BLoggedOn())
			{
				m_CallbackHandle = SteamRemoteStorage()->UGCDownload(*(UGCHandle_t*)m_SharedHandle.GetBytes(), 0);
			}
			else
			{
				LogSteamCoreWarn("Steam user not logged in.");
			}
		}
		else
		{
			LogSteamCoreWarn("Steam remote storage API disabled.");
		}

		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false) &&
				((m_CallbackResults.m_eResult == k_EResultOK) ? true : false);
		}
	}
	else
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreReadSharedFile::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	const FOnlineSharedCloudSteamCorePtr SharedCloud = StaticCastSharedPtr<FOnlineSharedCloudSteamCore>(Subsystem->GetSharedCloudInterface());
	if (FCloudFileSteam* SharedFile = SharedCloud->GetSharedCloudFile(m_SharedHandle))
	{
		if (bWasSuccessful)
		{
			if (FSharedContentHandleSteam(m_CallbackResults.m_hFile) == m_SharedHandle && m_CallbackResults.m_nSizeInBytes > 0 && m_CallbackResults.m_nSizeInBytes <= k_unMaxCloudFileChunkSize)
			{
				SharedFile->Data.Empty(m_CallbackResults.m_nSizeInBytes);
				SharedFile->Data.AddUninitialized(m_CallbackResults.m_nSizeInBytes);

				const uint32 FileOffset = 0;
				if (SteamRemoteStorage()->UGCRead(m_CallbackResults.m_hFile, SharedFile->Data.GetData(), SharedFile->Data.Num(), FileOffset, k_EUGCRead_ContinueReadingUntilFinished) != m_CallbackResults.m_nSizeInBytes)
				{
					SharedFile->Data.Empty();
					bWasSuccessful = false;
				}
			}
			else
			{
				bWasSuccessful = false;
			}
		}

		SharedFile->AsyncState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
	}
	else
	{
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreReadSharedFile::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const IOnlineSharedCloudPtr SharedCloudInterface = Subsystem->GetSharedCloudInterface();
	SharedCloudInterface->TriggerOnReadSharedFileCompleteDelegates(bWasSuccessful, m_SharedHandle);
}

FString FOnlineAsyncTaskSteamCoreWriteSharedFile::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreWriteSharedFile bWasSuccessful:%d UserId:%s FileName:%s Handle:%s"), WasSuccessful(), *m_UserId->ToDebugString(), *m_FileName, *FSharedContentHandleSteam(m_CallbackResults.m_hFile).ToDebugString());
}

void FOnlineAsyncTaskSteamCoreWriteSharedFile::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		if (WriteUserFile(*m_UserId, m_FileName, m_Contents))
		{
			m_CallbackHandle = SteamRemoteStorage()->FileShare(TCHAR_TO_UTF8(*m_FileName));
		}

		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false) &&
				((m_CallbackResults.m_eResult == k_EResultOK) ? true : false);
		}
	}
	else
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreWriteSharedFile::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	if (bWasSuccessful)
	{
		const FOnlineSharedCloudSteamCorePtr SharedCloud = StaticCastSharedPtr<FOnlineSharedCloudSteamCore>(Subsystem->GetSharedCloudInterface());
		const FSharedContentHandleSteam SharedHandle(m_CallbackResults.m_hFile);

		if (FCloudFileSteam* SharedFile = SharedCloud->GetSharedCloudFile(SharedHandle))
		{
			SharedFile->Data = m_Contents;
			SharedFile->AsyncState = EOnlineAsyncTaskState::Done;
		}
	}

	m_Contents.Empty();
}

void FOnlineAsyncTaskSteamCoreWriteSharedFile::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const IOnlineSharedCloudPtr SharedCloudInterface = Subsystem->GetSharedCloudInterface();

	const UGCHandle_t NewHandle = bWasSuccessful ? m_CallbackResults.m_hFile : k_UGCHandleInvalid;
	const TSharedRef<FSharedContentHandle> SharedHandle = MakeShareable(new FSharedContentHandleSteam(NewHandle));

	SharedCloudInterface->TriggerOnWriteSharedFileCompleteDelegates(bWasSuccessful, *m_UserId, m_FileName, SharedHandle);
}

FString FOnlineAsyncTaskSteamCoreEnumerateUserFiles::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreEnumerateUserFiles bWasSuccessful:%d UserId:%s"), WasSuccessful(), *m_UserId->ToDebugString());
}

void FOnlineAsyncTaskSteamCoreEnumerateUserFiles::Tick()
{
	LogSteamCoreVerbose("");
	bIsComplete = true;
	bWasSuccessful = false;

	if (SteamRemoteStorage())
	{
		if (SteamUser()->BLoggedOn() && SteamUser()->GetSteamID() == *m_UserId)
		{
			FScopeLock ScopeLock(&Subsystem->m_UserCloudDataLock);
			FSteamUserCloudData* UserMetadata = Subsystem->GetUserCloudEntry(*m_UserId);

			UserMetadata->m_CloudMetadata.Empty();

			const int32 FileCount = (int32)SteamRemoteStorage()->GetFileCount();
			for (int32 FileIdx = 0; FileIdx < FileCount; FileIdx++)
			{
				int32 FileSize = 0;
				const char* FileName = SteamRemoteStorage()->GetFileNameAndSize(FileIdx, &FileSize);
				new(UserMetadata->m_CloudMetadata) FCloudFileHeader(UTF8_TO_TCHAR(FileName), UTF8_TO_TCHAR(FileName), int32(FileSize));
			}

			bWasSuccessful = true;
		}
		else
		{
			LogSteamCoreWarn("Can only enumerate cloud files for logged in user.");
		}
	}
	else
	{
		LogSteamCoreWarn("Steam remote storage API disabled.");
	}
}

void FOnlineAsyncTaskSteamCoreEnumerateUserFiles::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	UserCloudInterface->TriggerOnEnumerateUserFilesCompleteDelegates(bWasSuccessful, *m_UserId);
}

FString FOnlineAsyncTaskSteamCoreReadUserFile::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreReadUserFile bWasSuccessful:%d UserId:%s FileName:%s"),WasSuccessful(), *m_UserId->ToDebugString(), *m_FileName);
}

void FOnlineAsyncTaskSteamCoreReadUserFile::Tick()
{
	LogSteamCoreVerbose("");
	bIsComplete = true;

	if (SteamRemoteStorage() && m_FileName.Len() > 0)
	{
		if (SteamUser()->BLoggedOn() && SteamUser()->GetSteamID() == *m_UserId)
		{
			const int32 FileSize = SteamRemoteStorage()->GetFileSize(TCHAR_TO_UTF8(*m_FileName));
			if (FileSize >= 0 && FileSize <= k_unMaxCloudFileChunkSize)
			{
				FScopeLock ScopeLock(&Subsystem->m_UserCloudDataLock);
				if (FSteamUserCloudData* UserCloud = Subsystem->GetUserCloudEntry(*m_UserId))
				{
					FCloudFile* UserCloudFile = UserCloud->GetFileData(m_FileName, true);
					check(UserCloudFile);

					UserCloudFile->Data.Empty(FileSize);
					UserCloudFile->Data.AddUninitialized(FileSize);
					if (SteamRemoteStorage()->FileRead(TCHAR_TO_UTF8(*m_FileName), UserCloudFile->Data.GetData(), FileSize) == FileSize)
					{
						bWasSuccessful = true;
					}
					else
					{
						UserCloudFile->Data.Empty();
					}
				}
			}
			else
			{
				LogSteamCoreWarn("Requested file %s has invalid size %d.", *m_FileName, FileSize);
			}
		}
		else
		{
			LogSteamCoreWarn("Can only read cloud files for logged in user.");
		}
	}
	else
	{
		LogSteamCoreWarn("Steam remote storage API disabled.");
	}

	{
		FScopeLock ScopeLock(&Subsystem->m_UserCloudDataLock);
		if (FSteamUserCloudData* UserCloud = Subsystem->GetUserCloudEntry(*m_UserId))
		{
			if (FCloudFile* UserCloudFileData = UserCloud->GetFileData(m_FileName))
			{
				UserCloudFileData->AsyncState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
			}
		}
	}
}

void FOnlineAsyncTaskSteamCoreReadUserFile::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	UserCloudInterface->TriggerOnReadUserFileCompleteDelegates(bWasSuccessful, *m_UserId, m_FileName);
}

bool FOnlineAsyncTaskSteamCoreWriteUserFile::WriteUserFile(const FUniqueNetId& InUserId, const FString& InFileToWrite, const TArray<uint8>& InContents)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;
	if (InFileToWrite.Len() > 0 && InContents.Num() > 0)
	{
		if (SteamRemoteStorage() && m_FileName.Len() > 0)
		{
			const CSteamID SteamId(*(uint64*)InUserId.GetBytes());
			if (SteamUser()->BLoggedOn() && SteamUser()->GetSteamID() == SteamId)
			{
				if (InContents.Num() < k_unMaxCloudFileChunkSize)
				{
					if (SteamRemoteStorage()->FileWrite(TCHAR_TO_UTF8(*InFileToWrite), InContents.GetData(), InContents.Num()))
					{
						FScopeLock ScopeLock(&Subsystem->m_UserCloudDataLock);
						if (FSteamUserCloudData* UserCloud = Subsystem->GetUserCloudEntry(InUserId))
						{
							FCloudFileHeader* UserCloudFileMetadata = UserCloud->GetFileMetadata(InFileToWrite, true);
							check(UserCloudFileMetadata);

							UserCloudFileMetadata->FileSize = SteamRemoteStorage()->GetFileSize(TCHAR_TO_UTF8(*InFileToWrite));
							UserCloudFileMetadata->Hash = FString(TEXT("0"));

							FCloudFile* UserCloudFileData = UserCloud->GetFileData(InFileToWrite, true);
							check(UserCloudFileData);

							UserCloudFileData->Data = InContents;
							bSuccess = true;
						}
					}
					else
					{
						LogSteamCoreWarn("Failed to write file to Steam cloud \"%s\".", *InFileToWrite);
					}
				}
				else
				{
					LogSteamCoreWarn("File too large %d to write to Steam cloud.", InContents.Num());
				}
			}
			else
			{
				LogSteamCoreWarn("Can only write cloud files for logged in user.");
			}
		}
		else
		{
			LogSteamCoreWarn("Steam remote storage API disabled.");
		}
	}

	{
		FScopeLock ScopeLock(&Subsystem->m_UserCloudDataLock);
		if (FSteamUserCloudData* UserCloud = Subsystem->GetUserCloudEntry(InUserId))
		{
			FCloudFile* UserCloudFileData = UserCloud->GetFileData(InFileToWrite, true);
			check(UserCloudFileData);
			UserCloudFileData->AsyncState = bSuccess ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
		}
	}

	return bSuccess;
}

FString FOnlineAsyncTaskSteamCoreWriteUserFile::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreWriteUserFile bWasSuccessful:%d UserId:%s FileName:%s"), WasSuccessful(), *m_UserId->ToDebugString(), *m_FileName);
}

void FOnlineAsyncTaskSteamCoreWriteUserFile::Tick()
{
	LogSteamCoreVerbose("");
	bIsComplete = true;
	if (WriteUserFile(*m_UserId, m_FileName, m_Contents))
	{
		bWasSuccessful = true;
	}

	m_Contents.Empty();
}

void FOnlineAsyncTaskSteamCoreWriteUserFile::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	UserCloudInterface->TriggerOnWriteUserFileCompleteDelegates(bWasSuccessful, *m_UserId, m_FileName);
}

FString FOnlineAsyncTaskSteamCoreDeleteUserFile::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreDeleteUserFile bWasSuccessful:%d UserId:%s FileName:%s"), WasSuccessful(), *m_UserId->ToDebugString(), *m_FileName);
}

void FOnlineAsyncTaskSteamCoreDeleteUserFile::Tick()
{
	LogSteamCoreVerbose("");
	bWasSuccessful = false;

	if (SteamRemoteStorage() && m_FileName.Len() > 0)
	{
		if (SteamUser()->BLoggedOn() && SteamUser()->GetSteamID() == *m_UserId)
		{
			bool bCloudDeleteSuccess = true;
			if (m_bShouldCloudDelete)
			{
				bCloudDeleteSuccess = SteamRemoteStorage()->FileForget(TCHAR_TO_UTF8(*m_FileName));
			}

			bool bLocalDeleteSuccess = true;
			if (m_bShouldLocallyDelete)
			{
				bLocalDeleteSuccess = false;
				const IOnlineUserCloudPtr UserCloud = Subsystem->GetUserCloudInterface();
				if (UserCloud->ClearFile(*m_UserId, m_FileName))
				{
					bLocalDeleteSuccess = SteamRemoteStorage()->FileDelete(TCHAR_TO_UTF8(*m_FileName));
					Subsystem->ClearUserCloudMetadata(*m_UserId, m_FileName);
				}
			}

			bWasSuccessful = bCloudDeleteSuccess && bLocalDeleteSuccess;
		}
		else
		{
			LogSteamCoreWarn("Can only delete cloud files for logged in user.");
		}
	}
	else
	{
		LogSteamCoreWarn("Steam remote storage API disabled.");
	}

	bIsComplete = true;
}

void FOnlineAsyncTaskSteamCoreDeleteUserFile::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	UserCloudInterface->TriggerOnDeleteUserFileCompleteDelegates(bWasSuccessful, *m_UserId, m_FileName);
}
#endif