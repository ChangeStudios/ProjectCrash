/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineLeaderboardTypesSteamCore.h"
#if UE_VERSION_OLDER_THAN(5,0,0)
#include "UObject/CoreOnline.h"
#else
#include "Online/CoreOnline.h"
#endif
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineStats.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "OnlineSubsystemSteamCorePackage.h"

class FOnlineSubsystemSteamCore;
DECLARE_DELEGATE_OneParam(FOnSteamUserStatsStoreStatsFinished, EOnlineAsyncTaskState::Type);

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineLeaderboardsSteamCore : public IOnlineLeaderboards
{
	friend class FOnlineAsyncTaskSteamCoreUpdateLeaderboard;
	friend class FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries;
	friend class FOnlineAsyncTaskSteamCoreRetrieveLeaderboard;
	friend class FOnlineAsyncTaskSteamCoreStoreStats;
	friend class FOnlineAsyncEventSteamStatsStored;
private:
	FOnlineLeaderboardsSteamCore()
		: m_SteamSubsystem(nullptr)
	{
	}

PACKAGE_SCOPE:
	FOnlineLeaderboardsSteamCore(FOnlineSubsystemSteamCore* InSteamSubsystem)
		: m_SteamSubsystem(InSteamSubsystem)
	{
	}

	FLeaderboardMetadataSteam* GetLeaderboardMetadata(const FString& LeaderboardName);

	void CreateLeaderboard(const FString& LeaderboardName, ELeaderboardSort::Type SortMethod, ELeaderboardFormat::Type DisplayFormat);
	void FindLeaderboard(const FString& LeaderboardName);
	static void CacheCurrentUsersStats();

	EOnlineAsyncTaskState::Type GetUserStatsState(const FUniqueNetIdSteam& UserId);
	void SetUserStatsState(const FUniqueNetIdSteam& UserId, EOnlineAsyncTaskState::Type NewState);

	void WriteAchievementsInternal(const FUniqueNetIdSteam& UserId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& OnWriteFinishedDelegate) const;
	void QueryAchievementsInternal(const FUniqueNetIdSteam& UserId, const FOnQueryAchievementsCompleteDelegate& AchievementDelegate) const;

public:
	virtual ~FOnlineLeaderboardsSteamCore() override
	{
	};

	// IOnlineLeaderboards
	virtual bool ReadLeaderboards(const TArray<FUniqueNetIdRef>& Players, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsForFriends(int32 LocalUserNum, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsAroundRank(int32 Rank, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual bool ReadLeaderboardsAroundUser(FUniqueNetIdRef Player, uint32 Range, FOnlineLeaderboardReadRef& ReadObject) override;
	virtual void FreeStats(FOnlineLeaderboardRead& ReadObject) override;
	virtual bool WriteLeaderboards(const FName& SessionName, const FUniqueNetId& Player, FOnlineLeaderboardWrite& WriteObject) override;
	virtual bool FlushLeaderboards(const FName& SessionName) override;
	virtual bool WriteOnlinePlayerRatings(const FName& SessionName, int32 LeaderboardId, const TArray<FOnlinePlayerScore>& PlayerScores) override;

private:
	FCriticalSection m_LeaderboardMetadataLock;
	FCriticalSection m_UserStatsStoredLock;
	FOnSteamUserStatsStoreStatsFinished m_UserStatsStoreStatsFinishedDelegate;
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	TArray<FLeaderboardMetadataSteam> m_Leaderboards;
};

typedef TSharedPtr<FOnlineLeaderboardsSteamCore, ESPMode::ThreadSafe> FOnlineLeaderboardsSteamCorePtr;
#endif
