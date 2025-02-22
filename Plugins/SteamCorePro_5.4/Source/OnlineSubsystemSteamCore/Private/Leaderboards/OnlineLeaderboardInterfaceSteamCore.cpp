/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Leaderboards/OnlineLeaderboardInterfaceSteamCore.h"
#include "Leaderboards/OnlineLeaderboardAsyncTasksSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE

bool FOnlineLeaderboardsSteamCore::ReadLeaderboards(const TArray<FUniqueNetIdRef>& Players, FOnlineLeaderboardReadRef& ReadObject)
{
	LogSteamCoreVerbose("");
	ReadObject->ReadState = EOnlineAsyncTaskState::InProgress;
	ReadObject->Rows.Empty();

#if UE_VERSION_OLDER_THAN(5,5,0)
	FString LeaderboardName = ReadObject->LeaderboardName.ToString();
#else
	FString LeaderboardName = ReadObject->LeaderboardName;
#endif
	
	FindLeaderboard(LeaderboardName);

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries* NewLeaderboardTask = new FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(m_SteamSubsystem, Players, ReadObject);
	m_SteamSubsystem->QueueAsyncTask(NewLeaderboardTask);

	const int32 NumPlayers = Players.Num();
	for (int32 UserIdx = 0; UserIdx < NumPlayers; UserIdx++)
	{
		const bool bLastPlayer = (UserIdx == NumPlayers - 1) ? true : false;
		const FUniqueNetIdSteam& UserId = FUniqueNetIdSteam::Cast(*Players[UserIdx]);
		FOnlineAsyncTaskSteamCoreRetrieveStats* NewStatsTask = new FOnlineAsyncTaskSteamCoreRetrieveStats(m_SteamSubsystem, UserId, ReadObject, bLastPlayer);
		m_SteamSubsystem->QueueAsyncTask(NewStatsTask);
	}

	return true;
}

bool FOnlineLeaderboardsSteamCore::ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject)
{
	LogSteamCoreVerbose("");
	ReadObject->ReadState = EOnlineAsyncTaskState::InProgress;

	ReadObject->Rows.Empty();

#if UE_VERSION_OLDER_THAN(5,5,0)
	FString LeaderboardName = ReadObject->LeaderboardName.ToString();
#else
	FString LeaderboardName = ReadObject->LeaderboardName;
#endif
	
	FindLeaderboard(LeaderboardName);

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries* NewLeaderboardTask = new FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(m_SteamSubsystem, Rank, Range, ReadObject);
	m_SteamSubsystem->QueueAsyncTask(NewLeaderboardTask);

	return true;
}

bool FOnlineLeaderboardsSteamCore::ReadLeaderboardsAroundUser(FUniqueNetIdRef Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject)
{
	LogSteamCoreVerbose("");
	ReadObject->ReadState = EOnlineAsyncTaskState::InProgress;
	ReadObject->Rows.Empty();

#if UE_VERSION_OLDER_THAN(5,5,0)
	FString LeaderboardName = ReadObject->LeaderboardName.ToString();
#else
	FString LeaderboardName = ReadObject->LeaderboardName;
#endif
	
	FindLeaderboard(LeaderboardName);

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries* NewLeaderboardTask = new FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(m_SteamSubsystem, Player, Range, ReadObject);

	m_SteamSubsystem->QueueAsyncTask(NewLeaderboardTask);

	return true;
}

bool FOnlineLeaderboardsSteamCore::ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject)
{
	LogSteamCoreVerbose("");
	ReadObject->ReadState = EOnlineAsyncTaskState::InProgress;

	ReadObject->Rows.Empty();

#if UE_VERSION_OLDER_THAN(5,5,0)
	FString LeaderboardName = ReadObject->LeaderboardName.ToString();
#else
	FString LeaderboardName = ReadObject->LeaderboardName;
#endif
	
	FindLeaderboard(LeaderboardName);

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries* NewLeaderboardTask = new FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(m_SteamSubsystem, ReadObject);
	m_SteamSubsystem->QueueAsyncTask(NewLeaderboardTask);

	return true;
}

void FOnlineLeaderboardsSteamCore::QueryAchievementsInternal(const FUniqueNetIdSteam& UserId, const FOnQueryAchievementsCompleteDelegate& AchievementDelegate) const
{
	FOnlineAsyncTaskSteamCoreGetAchievements* NewStatsTask = new FOnlineAsyncTaskSteamCoreGetAchievements(m_SteamSubsystem, UserId, AchievementDelegate);

	m_SteamSubsystem->QueueAsyncTask(NewStatsTask);
}


void FOnlineLeaderboardsSteamCore::FreeStats(FOnlineLeaderboardRead& ReadObject)
{
	LogSteamCoreVerbose("");
}

bool FOnlineLeaderboardsSteamCore::WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject)
{
	LogSteamCoreVerbose("");
	const bool bWasSuccessful = true;

	const int32 NumLeaderboards = WriteObject.LeaderboardNames.Num();
	for (int32 LeaderboardIdx = 0; LeaderboardIdx < NumLeaderboards; LeaderboardIdx++)
	{
#if UE_VERSION_OLDER_THAN(5,5,0)
		FString LeaderboardName = WriteObject.LeaderboardNames[LeaderboardIdx].ToString();
#else
		FString LeaderboardName = WriteObject.LeaderboardNames[LeaderboardIdx];
#endif
		CreateLeaderboard(LeaderboardName, WriteObject.SortMethod, WriteObject.DisplayFormat);
	}

	FStatPropertyArray LeaderboardStats;
	for (int32 LeaderboardIdx = 0; LeaderboardIdx < NumLeaderboards; LeaderboardIdx++)
	{
		for (FStatPropertyArray::TConstIterator It(WriteObject.Properties); It; ++It)
		{
			const FVariantData& Stat = It.Value();
#if UE_VERSION_OLDER_THAN(5,5,0)
			FString LeaderboardStatName = GetLeaderboardStatName(WriteObject.LeaderboardNames[LeaderboardIdx].ToString(), It.Key().ToString());
			LeaderboardStats.Add(FName(*LeaderboardStatName), Stat);
#else
			FString LeaderboardStatName = GetLeaderboardStatName(WriteObject.LeaderboardNames[LeaderboardIdx], It.Key());
			LeaderboardStats.Add(LeaderboardStatName, Stat);
#endif
		}
	}

	const FUniqueNetIdSteam& UserId = FUniqueNetIdSteam::Cast(Player);
	FOnlineAsyncTaskSteamCoreUpdateStats* NewUpdateStatsTask = new FOnlineAsyncTaskSteamCoreUpdateStats(m_SteamSubsystem, UserId, LeaderboardStats);
	m_SteamSubsystem->QueueAsyncTask(NewUpdateStatsTask);

	for (int32 LeaderboardIdx = 0; LeaderboardIdx < NumLeaderboards; LeaderboardIdx++)
	{
		const bool bLastLeaderboard = (LeaderboardIdx == NumLeaderboards - 1) ? true : false;

#if UE_VERSION_OLDER_THAN(5,5,0)
		FString LeaderboardName = WriteObject.LeaderboardNames[LeaderboardIdx].ToString();
		FString RatedStat = WriteObject.RatedStat.ToString();
#else
		FString LeaderboardName = WriteObject.LeaderboardNames[LeaderboardIdx];
		FString RatedStat = WriteObject.RatedStat;
#endif
		FOnlineAsyncTaskSteamCoreUpdateLeaderboard* NewUpdateLeaderboardTask = new FOnlineAsyncTaskSteamCoreUpdateLeaderboard(m_SteamSubsystem, LeaderboardName, RatedStat, WriteObject.UpdateMethod, bLastLeaderboard);
		m_SteamSubsystem->QueueAsyncTask(NewUpdateLeaderboardTask);
	}

	return bWasSuccessful;
}

void FOnlineLeaderboardsSteamCore::WriteAchievementsInternal(const FUniqueNetIdSteam& UserId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& OnWriteFinishedDelegate) const
{
	FOnlineAsyncTaskSteamCoreWriteAchievements* NewTask = new FOnlineAsyncTaskSteamCoreWriteAchievements(m_SteamSubsystem, UserId, WriteObject, OnWriteFinishedDelegate);
	m_SteamSubsystem->QueueAsyncTask(NewTask);
}

bool FOnlineLeaderboardsSteamCore::FlushLeaderboards(const FName& SessionName)
{
	LogSteamCoreVerbose("");
	const FUniqueNetIdSteamRef UserId = FUniqueNetIdSteam::Create(SteamUser()->GetSteamID());
	FOnlineAsyncTaskSteamCoreFlushLeaderboards* NewTask = new FOnlineAsyncTaskSteamCoreFlushLeaderboards(m_SteamSubsystem, SessionName, *UserId);
	m_SteamSubsystem->QueueAsyncTask(NewTask);
	return true;
}

bool FOnlineLeaderboardsSteamCore::WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores)
{
	LogSteamCoreVerbose("");
	return false;
}

FLeaderboardMetadataSteam* FOnlineLeaderboardsSteamCore::GetLeaderboardMetadata(const FString& LeaderboardName)
{
	LogSteamCoreVerbose("");
	FScopeLock ScopeLock(&m_LeaderboardMetadataLock);
	for (int32 LeaderboardIdx = 0; LeaderboardIdx < m_Leaderboards.Num(); LeaderboardIdx++)
	{
		const FLeaderboardMetadataSteam& Leaderboard = m_Leaderboards[LeaderboardIdx];
		if (Leaderboard.m_LeaderboardName == LeaderboardName)
		{
			return &m_Leaderboards[LeaderboardIdx];
		}
	}

	return nullptr;
}

void FOnlineLeaderboardsSteamCore::CreateLeaderboard(const FString& LeaderboardName, ELeaderboardSort::Type SortMethod, ELeaderboardFormat::Type DisplayFormat)
{
	LogSteamCoreVerbose("");
	FScopeLock ScopeLock(&m_LeaderboardMetadataLock);
	const FLeaderboardMetadataSteam* LeaderboardMetadata = GetLeaderboardMetadata(LeaderboardName);

	const bool bPrevAttemptFailed = (LeaderboardMetadata != nullptr && LeaderboardMetadata->m_LeaderboardHandle == -1 &&
		(LeaderboardMetadata->m_AsyncState == EOnlineAsyncTaskState::Done ||
			LeaderboardMetadata->m_AsyncState == EOnlineAsyncTaskState::Failed));

	if (LeaderboardMetadata == nullptr || bPrevAttemptFailed)
	{
		FLeaderboardMetadataSteam* NewLeaderboard = new(m_Leaderboards) FLeaderboardMetadataSteam(LeaderboardName, SortMethod, DisplayFormat);
		NewLeaderboard->m_AsyncState = EOnlineAsyncTaskState::InProgress;
		m_SteamSubsystem->QueueAsyncTask(new FOnlineAsyncTaskSteamCoreRetrieveLeaderboard(m_SteamSubsystem, LeaderboardName, SortMethod, DisplayFormat));
	}
}

void FOnlineLeaderboardsSteamCore::FindLeaderboard(const FString& LeaderboardName)
{
	LogSteamCoreVerbose("");
	FScopeLock ScopeLock(&m_LeaderboardMetadataLock);
	const FLeaderboardMetadataSteam* LeaderboardMetadata = GetLeaderboardMetadata(LeaderboardName);

	const bool bPrevAttemptFailed = (LeaderboardMetadata != nullptr && LeaderboardMetadata->m_LeaderboardHandle == -1 &&
		(LeaderboardMetadata->m_AsyncState == EOnlineAsyncTaskState::Done ||
			LeaderboardMetadata->m_AsyncState == EOnlineAsyncTaskState::Failed));

	if (LeaderboardMetadata == nullptr || bPrevAttemptFailed)
	{
		FLeaderboardMetadataSteam* NewLeaderboard = new(m_Leaderboards) FLeaderboardMetadataSteam(LeaderboardName);
		NewLeaderboard->m_AsyncState = EOnlineAsyncTaskState::InProgress;
		m_SteamSubsystem->QueueAsyncTask(new FOnlineAsyncTaskSteamCoreRetrieveLeaderboard(m_SteamSubsystem, LeaderboardName));
	}
}

void FOnlineLeaderboardsSteamCore::CacheCurrentUsersStats()
{
	LogSteamCoreVerbose("");
	// RequestCurrentStats is deprecated in steamworks sdk 1.61
//	SteamUserStats()->RequestCurrentStats();
}
#endif