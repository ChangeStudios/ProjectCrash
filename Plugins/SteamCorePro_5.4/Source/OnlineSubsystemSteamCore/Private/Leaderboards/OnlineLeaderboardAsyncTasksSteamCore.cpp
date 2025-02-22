/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Leaderboards/OnlineLeaderboardAsyncTasksSteamCore.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE

void FOnlineAsyncTaskSteamCoreRequestUserStats::Tick()
{
	LogSteamCoreVerbose("");
	if (!m_bInit)
	{
		ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
		check(SteamUserStatsPtr);
		m_CallbackHandle = SteamUserStatsPtr->RequestUserStats(*(uint64*)m_UserId->GetBytes());
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		ISteamUtils* SteamUtilsPtr = SteamUtils();
		check(SteamUtilsPtr);
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false);
		}
	}
	else
	{
		bWasSuccessful = false;
		bIsComplete = false;
	}
}

void FOnlineAsyncTaskSteamCoreRequestUserStats::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	if (bWasSuccessful)
	{
		const CGameID GameID(Subsystem->GetSteamAppId());
		if (GameID.ToUint64() == m_CallbackResults.m_nGameID)
		{
			check(m_CallbackResults.m_steamIDUser == *m_UserId);
			if (m_CallbackResults.m_eResult != k_EResultOK)
			{
				if (m_CallbackResults.m_eResult == k_EResultFail)
				{
					LogSteamCoreWarn("Failed to obtain steam user stats, user: %s has no stats entries", *m_UserId->ToDebugString());
				}
				else
				{
					LogSteamCoreWarn("Failed to obtain steam user stats, user: %s error: %s", *m_UserId->ToDebugString(), *SteamCore::SteamResultString(m_CallbackResults.m_eResult));
				}
			}

			bWasSuccessful = (m_CallbackResults.m_eResult == k_EResultOK) ? true : false;
		}
		else
		{
			LogSteamCoreWarn("Obtained steam user stats, but for wrong game! Ignoring.");
		}
	}
	else
	{
		LogSteamCoreWarn("Failed to obtain steam user stats, user: %s error: unknown", *m_UserId->ToDebugString());
	}
}

void FOnlineAsyncTaskSteamCoreUpdateStats::Tick()
{
	LogSteamCoreVerbose("");
	if (!m_bInit)
	{
		m_CallbackHandle = SteamUserStats()->RequestUserStats(*m_UserId);
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		ISteamUtils* SteamUtilsPtr = SteamUtils();
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

			if (bWasSuccessful)
			{
				ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
				check(SteamUserStatsPtr);
				for (FStatPropertyArray::TConstIterator It(m_Stats); It; ++It)
				{
					bool bSuccess = false;
#if UE_VERSION_OLDER_THAN(5,5,0)
					const FString StatName = It.Key().ToString();
#else
					const FString StatName = It.Key();
#endif
					const FVariantData& Stat = It.Value();

					switch (Stat.GetType())
					{
					case EOnlineKeyValuePairDataType::Int32:
						{
							int32 Value;
							Stat.GetValue(Value);
							bSuccess = SteamUserStatsPtr->SetStat(TCHAR_TO_UTF8(*StatName), Value) ? true : false;
							break;
						}

					case EOnlineKeyValuePairDataType::Float:
						{
							float Value;
							Stat.GetValue(Value);
							bSuccess = SteamUserStatsPtr->SetStat(TCHAR_TO_UTF8(*StatName), Value) ? true : false;
							break;
						}
					default:
						LogSteamCoreWarn("Skipping unsuppported key value pair uploading to Steam %s=%s", *StatName, *Stat.ToString());
						break;
					}

					if (!bSuccess)
					{
						LogSteamCoreWarn("Failure to write key value pair when uploading to Steam %s=%s", *StatName, *Stat.ToString());
						bWasSuccessful = false;
					}
				}
			}
		}
	}
	else
	{
		bWasSuccessful = false;
		bIsComplete = true;
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveStats::Tick()
{
	LogSteamCoreVerbose("");
	if (!m_bInit)
	{
		m_CallbackHandle = SteamUserStats()->RequestUserStats(*m_UserId);
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		ISteamUtils* SteamUtilsPtr = SteamUtils();
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false);
		}
	}
	else
	{
		bWasSuccessful = false;
		bIsComplete = true;
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveStats::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

		ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
		check(SteamUserStatsPtr);

		FOnlineStatsRow* UserRow = m_ReadObject->FindPlayerRecord(*m_UserId);
		if (UserRow == nullptr)
		{
			const FString NickName(UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(*m_UserId)));
			UserRow = new(m_ReadObject->Rows) FOnlineStatsRow(NickName, m_UserId);
		}

		if (bWasSuccessful)
		{
			if (m_CallbackResults.m_eResult != k_EResultOK)
			{
				for (int32 StatIdx = 0; StatIdx < m_ReadObject->ColumnMetadata.Num(); StatIdx++)
				{
					FVariantData EmptyData;
					const FColumnMetaData& ColumnMeta = m_ReadObject->ColumnMetadata[StatIdx];
					UserRow->Columns.Add(ColumnMeta.ColumnName, EmptyData);
				}
			}
			else
			{
				for (int32 StatIdx = 0; StatIdx < m_ReadObject->ColumnMetadata.Num(); StatIdx++)
				{
					const FColumnMetaData& ColumnMeta = m_ReadObject->ColumnMetadata[StatIdx];
#if UE_VERSION_OLDER_THAN(5,5,0)
					FString LeaderboardStat = GetLeaderboardStatName(m_ReadObject->LeaderboardName.ToString(), ColumnMeta.ColumnName.ToString());
#else
					FString LeaderboardStat = GetLeaderboardStatName(m_ReadObject->LeaderboardName, ColumnMeta.ColumnName);
#endif
					const FString StatName = LeaderboardStat;

					bool bSuccess = false;
					FVariantData* LastColumn = nullptr;
					switch (ColumnMeta.DataType)
					{
					case EOnlineKeyValuePairDataType::Int32:
						{
							int32 Value;
							bSuccess = SteamUserStatsPtr->GetUserStat(*m_UserId, TCHAR_TO_UTF8(*StatName), &Value) ? true : false;
							LastColumn = &(UserRow->Columns.Add(ColumnMeta.ColumnName, FVariantData(Value)));
							break;
						}

					case EOnlineKeyValuePairDataType::Float:
						{
							float Value;
							bSuccess = SteamUserStatsPtr->GetUserStat(*m_UserId, TCHAR_TO_UTF8(*StatName), &Value) ? true : false;
							LastColumn = &(UserRow->Columns.Add(ColumnMeta.ColumnName, FVariantData(Value)));
							break;
						}
					default:
						LogSteamCoreWarn("Unsupported key value pair during retrieval from Steam %s", *StatName);
						LastColumn = &(UserRow->Columns.Add(ColumnMeta.ColumnName, FVariantData()));
						break;
					}

					if (!bSuccess)
					{
						LogSteamCoreWarn("Failure to read key value pair during retrieval from Steam %s", *StatName);
						LastColumn->Empty();
						bWasSuccessful = false;
					}
				}
			}
		}

		m_ReadObject->ReadState = (bWasSuccessful && m_ReadObject->ReadState != EOnlineAsyncTaskState::Failed) ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
}

void FOnlineAsyncTaskSteamCoreRetrieveStats::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	if (m_bShouldTriggerDelegates)
	{
		const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
		Leaderboards->TriggerOnLeaderboardReadCompleteDelegates(m_ReadObject->ReadState == EOnlineAsyncTaskState::Done ? true : false);
	}
}

void FOnlineAsyncTaskSteamCoreStoreStats::Tick()
{
	LogSteamCoreVerbose("");
	const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());

	if (!m_bInit)
	{
		OperationStarted();
		m_bInit = true;
		Leaderboards->m_UserStatsStoreStatsFinishedDelegate.BindRaw(this, &FOnlineAsyncTaskSteamCoreStoreStats::OnUserStatsStoreStatsFinished);
		if (!SteamUserStats()->StoreStats())
		{
			OnUserStatsStoreStatsFinished(EOnlineAsyncTaskState::Failed);
			return;
		}
	}
}

void FOnlineAsyncTaskSteamCoreStoreStats::OnUserStatsStoreStatsFinished(EOnlineAsyncTaskState::Type State)
{
	LogSteamCoreVerbose("");
	const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
	Leaderboards->m_UserStatsStoreStatsFinishedDelegate.Unbind();

	bIsComplete = true;
	bWasSuccessful = State == EOnlineAsyncTaskState::Done;

	if (bWasSuccessful)
	{
		OperationSucceeded();
	}
	else
	{
		OperationFailed();
	}
}

void FOnlineAsyncTaskSteamCoreGetAchievements::Tick()
{
	LogSteamCoreVerbose("");
	if (!m_bInit)
	{
		m_CallbackHandle = SteamUserStats()->RequestUserStats(*m_UserId);
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		ISteamUtils* SteamUtilsPtr = SteamUtils();
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false) &&
				m_CallbackResults.m_eResult == k_EResultOK;
		}
	}
	else
	{
		bWasSuccessful = false;
		bIsComplete = true;
	}
}

void FOnlineAsyncTaskSteamCoreGetAchievements::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	const FOnlineAchievementsSteamCorePtr Achievements = StaticCastSharedPtr<FOnlineAchievementsSteamCore>(Subsystem->GetAchievementsInterface());
	Achievements->UpdateAchievementsForUser(*m_UserId, bWasSuccessful);
}

void FOnlineAsyncTaskSteamCoreGetAchievements::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	FOnlineAchievementsSteamCorePtr Achievements = StaticCastSharedPtr<FOnlineAchievementsSteamCore>(Subsystem->GetAchievementsInterface());
	m_AchievementDelegate.ExecuteIfBound(*m_UserId, bWasSuccessful);
}

void FOnlineAsyncTaskSteamCoreRetrieveLeaderboard::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		CreateOrFindLeaderboard(m_LeaderboardName, m_SortMethod, m_DisplayFormat);
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
				((m_CallbackResults.m_bLeaderboardFound != 0) ? true : false);
		}
	}
	else
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveLeaderboard::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());

	FScopeLock ScopeLock(&Leaderboards->m_LeaderboardMetadataLock);
	FLeaderboardMetadataSteam* Leaderboard = Leaderboards->GetLeaderboardMetadata(m_LeaderboardName);
	check(Leaderboard);

	if (bWasSuccessful)
	{
		ISteamUserStats* SteamUserPtr = SteamUserStats();
		check(m_LeaderboardName == FString(SteamUserPtr->GetLeaderboardName(m_CallbackResults.m_hSteamLeaderboard)));

		Leaderboard->m_LeaderboardHandle = m_CallbackResults.m_hSteamLeaderboard;
		Leaderboard->m_TotalLeaderboardRows = SteamUserPtr->GetLeaderboardEntryCount(m_CallbackResults.m_hSteamLeaderboard);
		Leaderboard->m_DisplayFormat = FromSteamLeaderboardDisplayType(SteamUserPtr->GetLeaderboardDisplayType(m_CallbackResults.m_hSteamLeaderboard));
		Leaderboard->m_SortMethod = FromSteamLeaderboardSortMethod(SteamUserPtr->GetLeaderboardSortMethod(m_CallbackResults.m_hSteamLeaderboard));
		Leaderboard->m_AsyncState = EOnlineAsyncTaskState::Done;
	}
	else
	{
		Leaderboard->m_LeaderboardHandle = -1;
		Leaderboard->m_TotalLeaderboardRows = 0;
		Leaderboard->m_AsyncState = EOnlineAsyncTaskState::Failed;
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveLeaderboard::CreateOrFindLeaderboard(const FString& InLeaderboardName, ELeaderboardSort::Type InSortMethod, ELeaderboardFormat::Type InDisplayFormat)
{
	LogSteamCoreVerbose("");
	if (m_bFindOnly)
	{
		m_CallbackHandle = SteamUserStats()->FindLeaderboard(TCHAR_TO_UTF8(*InLeaderboardName));
	}
	else
	{
		const ELeaderboardSortMethod SortMethodSteam = ToSteamLeaderboardSortMethod(InSortMethod);
		const ELeaderboardDisplayType DisplayTypeSteam = ToSteamLeaderboardDisplayType(InDisplayFormat);
		m_CallbackHandle = SteamUserStats()->FindOrCreateLeaderboard(TCHAR_TO_UTF8(*InLeaderboardName), SortMethodSteam, DisplayTypeSteam);
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries::Finalize()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::Finalize();

	TUniqueNetIdMap<int32> PlayersHaveStats;

	ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
	for (int32 EntryIdx = 0; EntryIdx < m_CallbackResults.m_cEntryCount; EntryIdx++)
	{
		LeaderboardEntry_t LeaderboardEntry;
		if (SteamUserStatsPtr->GetDownloadedLeaderboardEntry(m_CallbackResults.m_hSteamLeaderboardEntries, EntryIdx, &LeaderboardEntry, nullptr, 0))
		{
			FUniqueNetIdSteamRef CurrentUser = FUniqueNetIdSteam::Create(LeaderboardEntry.m_steamIDUser);
			FOnlineStatsRow* UserRow = m_ReadObject->FindPlayerRecord(*CurrentUser);
			if (UserRow == nullptr)
			{
				const FString NickName(UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(LeaderboardEntry.m_steamIDUser)));
				FUniqueNetIdRef UserId = FUniqueNetIdSteam::Create(LeaderboardEntry.m_steamIDUser);
				UserRow = new(m_ReadObject->Rows) FOnlineStatsRow(NickName, UserId);
			}

			UserRow->Rank = LeaderboardEntry.m_nGlobalRank;
			PlayersHaveStats.Add(CurrentUser, 1);

			if (m_Type != ERetrieveType::FetchUsers && m_Type != ERetrieveType::FetchRankUser)
			{
				FOnlineAsyncTaskSteamCoreRetrieveStats* NewStatsTask = new FOnlineAsyncTaskSteamCoreRetrieveStats(Subsystem, *CurrentUser,
				                                                                                                  m_ReadObject, (EntryIdx + 1 == m_CallbackResults.m_cEntryCount));

				Subsystem->QueueAsyncTask(NewStatsTask);
			}
		}
	}

	if (m_Type == ERetrieveType::FetchRankUser)
	{
		const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
		const FOnlineStatsRow* UserRow = m_ReadObject->FindPlayerRecord(*m_Players[0]);

		if (UserRow != nullptr)
		{
			Leaderboards->ReadLeaderboardsAroundRank(UserRow->Rank, m_Range, m_ReadObject);
			return;
		}
	}

	for (int32 UserIdx = 0; UserIdx < m_Players.Num(); UserIdx++)
	{
		const FUniqueNetIdRef& CurrentUser = m_Players[UserIdx];
		if (PlayersHaveStats.Find(CurrentUser) == nullptr)
		{
			FOnlineStatsRow* UserRow = m_ReadObject->FindPlayerRecord(*CurrentUser);
			if (UserRow == nullptr)
			{
				const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(*CurrentUser);
				const FString NickName(UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(SteamId)));
				UserRow = new(m_ReadObject->Rows) FOnlineStatsRow(NickName, SteamId.AsShared());

				if (m_Type != ERetrieveType::FetchUsers)
				{
					for (int32 StatIdx = 0; StatIdx < m_ReadObject->ColumnMetadata.Num(); StatIdx++)
					{
						FVariantData EmptyData;
						const FColumnMetaData& ColumnMeta = m_ReadObject->ColumnMetadata[StatIdx];
						UserRow->Columns.Add(ColumnMeta.ColumnName, EmptyData);
					}
				}
			}

			UserRow->Rank = -1;
		}
	}

	if (m_CallbackResults.m_cEntryCount == 0 && m_Type != ERetrieveType::FetchUsers)
	{
		m_ReadObject->ReadState = EOnlineAsyncTaskState::Done;
		m_bShouldTriggerDelegates = true;
	}
}

void FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	if (m_bShouldTriggerDelegates)
	{
		const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
		Leaderboards->TriggerOnLeaderboardReadCompleteDelegates(bWasSuccessful);
	}
}


void FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		SteamLeaderboard_t LeaderboardHandle = -1;
		{
			const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
			FScopeLock ScopeLock(&Leaderboards->m_LeaderboardMetadataLock);
			FString LeaderboardName = m_ReadObject->LeaderboardName.ToString();
			if (const FLeaderboardMetadataSteam* Leaderboard = Leaderboards->GetLeaderboardMetadata(LeaderboardName))
			{
				LeaderboardHandle = Leaderboard->m_LeaderboardHandle;
			}
		}

		if (LeaderboardHandle != -1)
		{
			ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
			switch (m_Type)
			{
			default:
			case ERetrieveType::FetchUsers:
			case ERetrieveType::FetchRankUser:
				{
					if (m_Players.Num() > 0)
					{
						const int32 NumUsers = FPlatformMath::Min(m_Players.Num(), 100);
						CSteamID* IdArray = new CSteamID[NumUsers];
						for (int32 UserIdx = 0; UserIdx < NumUsers; UserIdx++)
						{
							IdArray[UserIdx] = *(uint64*)m_Players[UserIdx]->GetBytes();
						}
						m_CallbackHandle = SteamUserStatsPtr->DownloadLeaderboardEntriesForUsers(LeaderboardHandle, IdArray, NumUsers);

						delete[] IdArray;
					}
				}
				break;
			case ERetrieveType::FetchCurRankUser:
				{
					m_CallbackHandle = SteamUserStatsPtr->DownloadLeaderboardEntries(LeaderboardHandle, k_ELeaderboardDataRequestGlobalAroundUser, -m_Range, m_Range);
				}
				break;
			case ERetrieveType::FetchRank:
				{
					const int32 AdjustedStart = ((m_Rank - m_Range) <= 0) ? 1 : (m_Rank - m_Range);
					m_CallbackHandle = SteamUserStatsPtr->DownloadLeaderboardEntries(LeaderboardHandle, k_ELeaderboardDataRequestGlobal, AdjustedStart, (m_Rank + m_Range));
				}
				break;
			case ERetrieveType::FetchFriends:
				{
					m_CallbackHandle = SteamUserStatsPtr->DownloadLeaderboardEntries(LeaderboardHandle, k_ELeaderboardDataRequestFriends, 0, 0);
				}
				break;
			}

			m_bInit = true;
		}
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
				((m_CallbackResults.m_hSteamLeaderboard != -1) ? true : false);
		}
	}
	else if (m_bInit)
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreUpdateLeaderboard::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		SteamLeaderboard_t LeaderboardHandle = -1;
		{
			const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
			FScopeLock ScopeLock(&Leaderboards->m_LeaderboardMetadataLock);
			if (const FLeaderboardMetadataSteam* Leaderboard = Leaderboards->GetLeaderboardMetadata(m_LeaderboardName))
			{
				LeaderboardHandle = Leaderboard->m_LeaderboardHandle;
			}
		}

		if (LeaderboardHandle != -1)
		{
			ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
			check(SteamUserStatsPtr);

			ELeaderboardUploadScoreMethod UpdateMethodSteam;
			switch (m_UpdateMethod)
			{
			case ELeaderboardUpdateMethod::Force:
				UpdateMethodSteam = k_ELeaderboardUploadScoreMethodForceUpdate;
				break;
			case ELeaderboardUpdateMethod::KeepBest:
			default:
				UpdateMethodSteam = k_ELeaderboardUploadScoreMethodKeepBest;
				break;
			}

			const FString RatedStatName = GetLeaderboardStatName(m_LeaderboardName, m_RatedStat);
			if (SteamUserStats()->GetStat(TCHAR_TO_UTF8(*RatedStatName), &m_NewScore))
			{
				m_CallbackHandle = SteamUserStatsPtr->UploadLeaderboardScore(LeaderboardHandle, UpdateMethodSteam, m_NewScore, nullptr, 0);
			}

			m_bInit = true;
		}
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
				((m_CallbackResults.m_bSuccess != 0) ? true : false);
		}
	}
	else if (m_bInit)
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreUpdateLeaderboard::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();
	if (m_bShouldTriggerDelegates)
	{
	}
}

void FOnlineAsyncTaskSteamCoreFlushLeaderboards::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
	Leaderboards->TriggerOnLeaderboardFlushCompleteDelegates(m_SessionName, bWasSuccessful);
}

void FOnlineAsyncTaskSteamCoreWriteAchievements::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTaskSteamCore::TriggerDelegates();

	const FOnlineAchievementsSteamCorePtr Achievements = StaticCastSharedPtr<FOnlineAchievementsSteamCore>(Subsystem->GetAchievementsInterface());
	Achievements->OnWriteAchievementsComplete(*m_UserId, bWasSuccessful, m_WriteObject, m_OnWriteFinishedDelegate);
}
#endif