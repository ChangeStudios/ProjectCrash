/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#if WITH_STEAMCORE
#include "isteamuserstats.h"
#endif
#include "OnlineSubsystemSteamCoreTypes.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FLeaderboardMetadataSteam
{
	friend class FOnlineLeaderboardsSteamCore;
private:
	FLeaderboardMetadataSteam()
		: m_SortMethod(ELeaderboardSort::Type::Ascending)
		  , m_DisplayFormat()
		  , m_TotalLeaderboardRows(0)
		  , m_LeaderboardHandle(0)
		  , m_AsyncState()
	{
	}

	FLeaderboardMetadataSteam(const FString& InLeaderboardName, ELeaderboardSort::Type InSortMethod, ELeaderboardFormat::Type InDisplayFormat)
		: m_LeaderboardName(InLeaderboardName),
		  m_SortMethod(InSortMethod),
		  m_DisplayFormat(InDisplayFormat),
		  m_TotalLeaderboardRows(0),
		  m_LeaderboardHandle(-1),
		  m_AsyncState(EOnlineAsyncTaskState::NotStarted)
	{
	}

	FLeaderboardMetadataSteam(const FString& InLeaderboardName)
		: m_LeaderboardName(InLeaderboardName),
		  m_SortMethod(ELeaderboardSort::None),
		  m_DisplayFormat(ELeaderboardFormat::Number),
		  m_TotalLeaderboardRows(0),
		  m_LeaderboardHandle(-1),
		  m_AsyncState(EOnlineAsyncTaskState::NotStarted)
	{
	}

public:
	FString m_LeaderboardName;
	ELeaderboardSort::Type m_SortMethod;
	ELeaderboardFormat::Type m_DisplayFormat;
	int32 m_TotalLeaderboardRows;
	SteamLeaderboard_t m_LeaderboardHandle;
	EOnlineAsyncTaskState::Type m_AsyncState;
};

struct ONLINESUBSYSTEMSTEAMCORE_API FUserStatsStateSteamCore
{
private:
	FUserStatsStateSteamCore() = delete;

public:
	FUserStatsStateSteamCore(const FUniqueNetIdSteam& InUserId, EOnlineAsyncTaskState::Type InState)
		: m_UserId(InUserId.AsShared()),
		  m_StatsState(InState)
	{
	}

	FUniqueNetIdSteamRef m_UserId;
	EOnlineAsyncTaskState::Type m_StatsState;
};
#endif