/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineStats.h"
#include "Interfaces/OnlineAchievementsInterface.h"

class FOnlineSubsystemSteamCore;

#if WITH_STEAMCORE

inline ELeaderboardSortMethod ToSteamLeaderboardSortMethod(ELeaderboardSort::Type InSortMethod)
{
	switch (InSortMethod)
	{
	case ELeaderboardSort::Ascending:
		return k_ELeaderboardSortMethodAscending;
	case ELeaderboardSort::Descending:
		return k_ELeaderboardSortMethodDescending;
	case ELeaderboardSort::None:
	default:
		return k_ELeaderboardSortMethodNone;
	}
}

inline ELeaderboardSort::Type FromSteamLeaderboardSortMethod(ELeaderboardSortMethod InSortMethod)
{
	switch (InSortMethod)
	{
	case k_ELeaderboardSortMethodAscending:
		return ELeaderboardSort::Ascending;
	case k_ELeaderboardSortMethodDescending:
		return ELeaderboardSort::Descending;
	case k_ELeaderboardSortMethodNone:
	default:
		return ELeaderboardSort::None;
	}
}

inline ELeaderboardDisplayType ToSteamLeaderboardDisplayType(ELeaderboardFormat::Type InDisplayFormat)
{
	switch (InDisplayFormat)
	{
	case ELeaderboardFormat::Seconds:
		return k_ELeaderboardDisplayTypeTimeSeconds;
	case ELeaderboardFormat::Milliseconds:
		return k_ELeaderboardDisplayTypeTimeMilliSeconds;
	case ELeaderboardFormat::Number:
	default:
		return k_ELeaderboardDisplayTypeNumeric;
	}
}

inline ELeaderboardFormat::Type FromSteamLeaderboardDisplayType(ELeaderboardDisplayType InDisplayFormat)
{
	switch (InDisplayFormat)
	{
	case k_ELeaderboardDisplayTypeTimeSeconds:
		return ELeaderboardFormat::Seconds;
	case k_ELeaderboardDisplayTypeTimeMilliSeconds:
		return ELeaderboardFormat::Milliseconds;
	case k_ELeaderboardDisplayTypeNumeric:
	default:
		return ELeaderboardFormat::Number;
	}
}

inline FString GetLeaderboardStatName(const FString& LeaderboardName, const FString& StatName)
{
	return TCHAR_TO_ANSI((*FString::Printf(TEXT("%s_%s"), *LeaderboardName, *StatName)));
}

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreRequestUserStats : public FOnlineAsyncTaskSteamCore
{
private:
	FOnlineAsyncTaskSteamCoreRequestUserStats() = delete;

public:
	FOnlineAsyncTaskSteamCoreRequestUserStats(FOnlineSubsystemSteamCore* InSteamSubsystem, const FUniqueNetIdSteam& InUserId)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_UserId(InUserId.AsShared()), m_CallbackResults()
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreRequestUserStats bWasSuccessful: %d UserId: %s"), WasSuccessful(), *m_UserId->ToDebugString());
	}

	virtual void Tick() override;
	virtual void Finalize() override;

private:
	bool m_bInit;
	FUniqueNetIdSteamRef m_UserId;
	UserStatsReceived_t m_CallbackResults;
};

class FOnlineAsyncTaskSteamCoreUpdateStats : public FOnlineAsyncTaskSteamCore
{
private:
	FOnlineAsyncTaskSteamCoreUpdateStats() = delete;

public:
	FOnlineAsyncTaskSteamCoreUpdateStats(FOnlineSubsystemSteamCore* InSteamSubsystem, const FUniqueNetIdSteam& InUserId, const FStatPropertyArray& InStats)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_UserId(InUserId.AsShared()),
		  m_Stats(InStats), m_CallbackResults()
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreUpdateStats bWasSuccessful: %d User: %s"), WasSuccessful(), *m_UserId->ToDebugString());
	}

	virtual void Tick() override;

private:
	bool m_bInit;
	FUniqueNetIdSteamRef m_UserId;
	const FStatPropertyArray m_Stats;
	UserStatsReceived_t m_CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreRetrieveStats : public FOnlineAsyncTaskSteamCore
{
public:
	FOnlineAsyncTaskSteamCoreRetrieveStats(FOnlineSubsystemSteamCore* InSteamSubsystem, const FUniqueNetIdSteam& InUserId, const FOnlineLeaderboardReadRef& InReadObject, bool bInShouldTriggerDelegates)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_UserId(InUserId.AsShared()),
		  m_ReadObject(InReadObject), m_CallbackResults(),
		  m_bShouldTriggerDelegates(bInShouldTriggerDelegates)
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreRetrieveStats bWasSuccessful: %d UserId: %s"), WasSuccessful(), *m_UserId->ToDebugString());
	}

	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bInit;
	FUniqueNetIdSteamRef m_UserId;
	FOnlineLeaderboardReadPtr m_ReadObject;
	UserStatsReceived_t m_CallbackResults;
	bool m_bShouldTriggerDelegates;
	FOnlineAsyncTaskSteamCoreRetrieveStats() = delete;
};

class FOnlineAsyncTaskSteamCoreStoreStats : public FOnlineAsyncTaskSteamCore
{
public:
	FOnlineAsyncTaskSteamCoreStoreStats()
		: FOnlineAsyncTaskSteamCore(nullptr, k_uAPICallInvalid),
		  m_bInit(false),
		  m_SessionName(NAME_None),
		  m_UserId(FUniqueNetIdSteam::EmptyId())
	{
	}

	virtual void OperationStarted()
	{
	}

	virtual void OperationFailed()
	{
	}

	virtual void OperationSucceeded()
	{
	}

public:
	FOnlineAsyncTaskSteamCoreStoreStats(FOnlineSubsystemSteamCore* InSteamSubsystem, const FName& InSessionName, const FUniqueNetIdSteam& InUserId)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_SessionName(InSessionName),
		  m_UserId(InUserId.AsShared())
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreStoreStats SessionName: %s bWasSuccessful: %d"), *m_SessionName.ToString(), WasSuccessful());
	}

	virtual void Tick() override;
	void OnUserStatsStoreStatsFinished(EOnlineAsyncTaskState::Type State);

protected:
	bool m_bInit;
	const FName m_SessionName;
	const FUniqueNetIdSteamRef m_UserId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreGetAchievements : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreGetAchievements() = delete;

public:
	FOnlineAsyncTaskSteamCoreGetAchievements(FOnlineSubsystemSteamCore* InSteamSubsystem, const FUniqueNetIdSteam& InUserId, const FOnQueryAchievementsCompleteDelegate& InAchievementDelegate)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_UserId(InUserId.AsShared()), m_CallbackResults(),
		  m_AchievementDelegate(InAchievementDelegate)
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreGetAchievements bWasSuccessful: %d UserId: %s"), WasSuccessful(), *m_UserId->ToDebugString());
	}

	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bInit;
	FUniqueNetIdSteamRef m_UserId;
	UserStatsReceived_t m_CallbackResults;
	FOnQueryAchievementsCompleteDelegate m_AchievementDelegate;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreRetrieveLeaderboard : public FOnlineAsyncTaskSteamCore
{
	FOnlineAsyncTaskSteamCoreRetrieveLeaderboard()
		: FOnlineAsyncTaskSteamCore(nullptr, k_uAPICallInvalid),
		  m_bInit(false),
		  m_SortMethod(ELeaderboardSort::Ascending),
		  m_DisplayFormat(ELeaderboardFormat::Number), m_CallbackResults(),
		  m_bFindOnly(true)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreRetrieveLeaderboard(FOnlineSubsystemSteamCore* InSteamSubsystem, const FString& InLeaderboardName, ELeaderboardSort::Type InSortMethod, ELeaderboardFormat::Type InDisplayFormat)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_LeaderboardName(InLeaderboardName),
		  m_SortMethod(InSortMethod),
		  m_DisplayFormat(InDisplayFormat), m_CallbackResults(),
		  m_bFindOnly(false)
	{
	}

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboard(FOnlineSubsystemSteamCore* InSteamSubsystem, const FString& InLeaderboardName)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_LeaderboardName(InLeaderboardName),
		  m_SortMethod(ELeaderboardSort::Ascending),
		  m_DisplayFormat(ELeaderboardFormat::Number), m_CallbackResults(),
		  m_bFindOnly(true)
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreRetrieveLeaderboard bWasSuccessful: %d"), WasSuccessful());
	}

	virtual void Tick() override;
	virtual void Finalize() override;

private:
	bool m_bInit;
	FString m_LeaderboardName;
	ELeaderboardSort::Type m_SortMethod;
	ELeaderboardFormat::Type m_DisplayFormat;
	LeaderboardFindResult_t m_CallbackResults;
	bool m_bFindOnly;

private:
	void CreateOrFindLeaderboard(const FString& InLeaderboardName, ELeaderboardSort::Type InSortMethod, ELeaderboardFormat::Type InDisplayFormat);
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries : public FOnlineAsyncTaskSteamCore
{
public:
	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(FOnlineSubsystemSteamCore* InSteamSubsystem, const TArray<FUniqueNetIdRef>& InPlayers, const FOnlineLeaderboardReadRef& InReadObject)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_Players(InPlayers),
		  m_ReadObject(InReadObject), m_CallbackResults(),
		  m_Type(ERetrieveType::FetchUsers), m_Rank(0), m_Range(0),
		  m_bShouldTriggerDelegates(false)
	{
	}

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(FOnlineSubsystemSteamCore* InSteamSubsystem, int32 InRank, int32 InRange, const FOnlineLeaderboardReadRef& InReadObject)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_ReadObject(InReadObject), m_CallbackResults(),
		  m_Type(ERetrieveType::FetchRank),
		  m_Rank(InRank),
		  m_Range(InRange),
		  m_bShouldTriggerDelegates(false)
	{
	}

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(FOnlineSubsystemSteamCore* InSteamSubsystem, const FOnlineLeaderboardReadRef& InReadObject)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_ReadObject(InReadObject), m_CallbackResults(),
		  m_Type(ERetrieveType::FetchFriends), m_Rank(0), m_Range(0),
		  m_bShouldTriggerDelegates(false)
	{
	}

	FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries(FOnlineSubsystemSteamCore* InSteamSubsystem, FUniqueNetIdRef InUser, int32 InRange, const FOnlineLeaderboardReadRef& InReadObject)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_ReadObject(InReadObject)
		  , m_CallbackResults()
		  , m_Type()
		  , m_Rank(0),
		  m_Range(InRange),
		  m_bShouldTriggerDelegates(false)
	{
		m_Players.Push(FUniqueNetIdSteam::Create(*InUser));
		m_Type = Subsystem->IsLocalPlayer(*m_Players[0]) ? ERetrieveType::FetchCurRankUser : ERetrieveType::FetchRankUser;
	}

	FString TaskTypeToString() const
	{
		switch (m_Type)
		{
		default:
			return TEXT("Invalid");
			break;
		case FetchUsers:
			return TEXT("Fetch Users");
			break;
		case FetchFriends:
			return TEXT("Fetch Friends");
			break;
		case FetchRank:
			return TEXT("Fetch Global Ranks");
			break;
		case FetchCurRankUser:
			return TEXT("Fetch Rank around current user");
			break;
		case FetchRankUser:
			return TEXT("Fetch Rank around the users");
			break;
		}
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreRetrieveLeaderboardEntries Task Type %s bWasSuccessful: %d"), *TaskTypeToString(), WasSuccessful());
	}

	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
private:
	enum ERetrieveType
	{
		None,
		FetchUsers,
		FetchFriends,
		FetchRank,
		FetchCurRankUser,
		FetchRankUser,
		Max
	};

	bool m_bInit;
	TArray<FUniqueNetIdRef> m_Players;
	FOnlineLeaderboardReadRef m_ReadObject;
	LeaderboardScoresDownloaded_t m_CallbackResults;
	ERetrieveType m_Type;
	int32 m_Rank;
	int32 m_Range;
	bool m_bShouldTriggerDelegates;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreUpdateLeaderboard : public FOnlineAsyncTaskSteamCore
{
private:
	FOnlineAsyncTaskSteamCoreUpdateLeaderboard()
		: FOnlineAsyncTaskSteamCore(nullptr, k_uAPICallInvalid),
		  m_bInit(false),
		  m_NewScore(0),
		  m_UpdateMethod(ELeaderboardUpdateMethod::KeepBest), m_CallbackResults(),
		  m_bShouldTriggerDelegates(false)
	{
	}

public:
	FOnlineAsyncTaskSteamCoreUpdateLeaderboard(FOnlineSubsystemSteamCore* InSteamSubsystem, const FString& InLeaderboardName, const FString& InRatedStat, ELeaderboardUpdateMethod::Type InUpdateMethod, bool bInShouldTriggerDelegates)
		: FOnlineAsyncTaskSteamCore(InSteamSubsystem, k_uAPICallInvalid),
		  m_bInit(false),
		  m_LeaderboardName(InLeaderboardName),
		  m_RatedStat(InRatedStat),
		  m_NewScore(0),
		  m_UpdateMethod(InUpdateMethod), m_CallbackResults(),
		  m_bShouldTriggerDelegates(bInShouldTriggerDelegates)
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreUpdateLeaderboard bWasSuccessful: %d Leaderboard: %s Score: %d"), WasSuccessful(), *m_LeaderboardName, m_NewScore);
	}

	virtual void Tick() override;
	virtual void TriggerDelegates() override;

private:
	bool m_bInit;
	FString m_LeaderboardName;
	FString m_RatedStat;
	int32 m_NewScore;
	ELeaderboardUpdateMethod::Type m_UpdateMethod;
	LeaderboardScoreUploaded_t m_CallbackResults;
	bool m_bShouldTriggerDelegates;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreFlushLeaderboards : public FOnlineAsyncTaskSteamCoreStoreStats
{
private:
	FOnlineAsyncTaskSteamCoreFlushLeaderboards()
		: FOnlineAsyncTaskSteamCoreStoreStats()
	{
	}

public:
	FOnlineAsyncTaskSteamCoreFlushLeaderboards(FOnlineSubsystemSteamCore* InSteamSubsystem, const FName& InSessionName, const FUniqueNetIdSteam& InUserId)
		: FOnlineAsyncTaskSteamCoreStoreStats(InSteamSubsystem, InSessionName, InUserId)
	{
	}

	virtual void TriggerDelegates() override;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreWriteAchievements : public FOnlineAsyncTaskSteamCoreStoreStats
{
private:
	FOnlineAsyncTaskSteamCoreWriteAchievements()
		: FOnlineAsyncTaskSteamCoreStoreStats(),
		  m_WriteObject(nullptr)
	{
	}

	virtual void OperationStarted() override
	{
		check(m_WriteObject.IsValid());
		m_WriteObject->WriteState = EOnlineAsyncTaskState::InProgress;
	}

	virtual void OperationFailed() override
	{
		check(m_WriteObject.IsValid());
		m_WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
	}

	virtual void OperationSucceeded() override
	{
		check(m_WriteObject.IsValid());
		m_WriteObject->WriteState = EOnlineAsyncTaskState::Done;
	}

private:
	FOnlineAchievementsWritePtr m_WriteObject;
	FOnAchievementsWrittenDelegate m_OnWriteFinishedDelegate;

public:
	FOnlineAsyncTaskSteamCoreWriteAchievements(FOnlineSubsystemSteamCore* InSteamSubsystem, const FUniqueNetIdSteam& InUserId, FOnlineAchievementsWriteRef& InWriteObject, const FOnAchievementsWrittenDelegate& InOnWriteFinishedDelegate)
		: FOnlineAsyncTaskSteamCoreStoreStats(InSteamSubsystem, TEXT("Unused"), InUserId)
		  , m_WriteObject(InWriteObject)
		  , m_OnWriteFinishedDelegate(InOnWriteFinishedDelegate)
	{
	}

	virtual void TriggerDelegates() override;
};
#endif