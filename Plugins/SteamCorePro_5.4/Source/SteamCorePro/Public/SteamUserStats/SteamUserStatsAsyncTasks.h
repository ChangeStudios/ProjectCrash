/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUserStats
*/

#pragma once

#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamUserStatsTypes.h"

#if WITH_STEAMCORE

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnFindLeaderboard m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard(const FOnFindLeaderboard Callback, const FString LeaderboardName, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_LeaderboardName(LeaderboardName)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard(USteamCoreProAsyncAction* AsyncObject, const FString LeaderboardName, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_LeaderboardName(LeaderboardName)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard() = delete;
protected:
	LeaderboardFindResult_t m_CallbackResults;
	FString m_LeaderboardName;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnDownloadLeaderboardEntries m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries(const FOnDownloadLeaderboardEntries Callback, const FSteamLeaderboard Leaderboard, const ESteamLeaderboardDataRequest DataRequest, const int32 RangeStart, const int32 RangeEnd, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_DataRequest(DataRequest)
		  , m_RangeStart(RangeStart)
		  , m_RangeEnd(RangeEnd)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries(USteamCoreProAsyncAction* AsyncObject, FSteamLeaderboard Leaderboard, const ESteamLeaderboardDataRequest DataRequest, const int32 RangeStart, const int32 RangeEnd, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_DataRequest(DataRequest)
		  , m_RangeStart(RangeStart)
		  , m_RangeEnd(RangeEnd)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries() = delete;
protected:
	LeaderboardScoresDownloaded_t m_CallbackResults;
	FSteamLeaderboard m_Leaderboard;
	ESteamLeaderboardDataRequest m_DataRequest;
	int32 m_RangeStart;
	int32 m_RangeEnd;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestGlobalStats m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats(const FOnRequestGlobalStats Callback, const int32 HistoryDays, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_HistoryDays(HistoryDays)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats(USteamCoreProAsyncAction* AsyncObject, const int32 HistoryDays, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_HistoryDays(HistoryDays)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats() = delete;
protected:
	GlobalStatsReceived_t m_CallbackResults;
	int32 m_HistoryDays;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestGlobalAchievementPercentages m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages(const FOnRequestGlobalAchievementPercentages Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages() = delete;
protected:
	GlobalAchievementPercentagesReady_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnGetNumberOfCurrentPlayers m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers(const FOnGetNumberOfCurrentPlayers Callback, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers(USteamCoreProAsyncAction* AsyncObject, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers() = delete;
protected:
	NumberOfCurrentPlayers_t m_CallbackResults;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnUploadLeaderboardScore m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore(const FOnUploadLeaderboardScore Callback, const FSteamLeaderboard Leaderboard, const ESteamLeaderboardUploadScoreMethod UploadMethod, const int32 Score, const TArray<int32> ScoreDetails, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_UploadScoreMethod(UploadMethod)
		  , m_Score(Score)
		  , m_ScoreDetails(ScoreDetails)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore(USteamCoreProAsyncAction* AsyncObject, const FSteamLeaderboard Leaderboard, const ESteamLeaderboardUploadScoreMethod UploadMethod, const int32 Score, const TArray<int32> ScoreDetails, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_UploadScoreMethod(UploadMethod)
		  , m_Score(Score)
		  , m_ScoreDetails(ScoreDetails)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore() = delete;
protected:
	LeaderboardScoreUploaded_t m_CallbackResults;
	FSteamLeaderboard m_Leaderboard;
	ESteamLeaderboardUploadScoreMethod m_UploadScoreMethod;
	int32 m_Score;
	TArray<int32> m_ScoreDetails;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnFindOrCreateLeaderboard m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard(const FOnFindOrCreateLeaderboard Callback, const FString LeaderboardName, const ESteamLeaderboardSortMethod SortMethod, const ESteamLeaderboardDisplayType DisplayType, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_LeaderboardName(LeaderboardName)
		  , m_SortMethod(SortMethod)
		  , m_DisplayType(DisplayType)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard(USteamCoreProAsyncAction* AsyncObject, const FString LeaderboardName, const ESteamLeaderboardSortMethod SortMethod, const ESteamLeaderboardDisplayType DisplayType, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_LeaderboardName(LeaderboardName)
		  , m_SortMethod(SortMethod)
		  , m_DisplayType(DisplayType)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard() = delete;
protected:
	LeaderboardFindResult_t m_CallbackResults;
	FString m_LeaderboardName;
	ESteamLeaderboardSortMethod m_SortMethod = ESteamLeaderboardSortMethod::None;
	ESteamLeaderboardDisplayType m_DisplayType = ESteamLeaderboardDisplayType::None;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnRequestUserStats m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats(const FOnRequestUserStats Callback, const FSteamID SteamID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_SteamID(SteamID)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats(USteamCoreProAsyncAction* AsyncObject, const FSteamID SteamID, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_SteamID(SteamID)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats() = delete;
protected:
	UserStatsReceived_t m_CallbackResults;
	FSteamID m_SteamID;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnDownloadLeaderboardEntriesForUsers m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers(const FOnDownloadLeaderboardEntriesForUsers Callback, const FSteamLeaderboard Leaderboard, const TArray<FSteamID> Users, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_Users(Users)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers(USteamCoreProAsyncAction* AsyncObject, const FSteamLeaderboard Leaderboard, const TArray<FSteamID> Users, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_Users(Users)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers() = delete;
protected:
	LeaderboardScoresDownloaded_t m_CallbackResults;
	FSteamLeaderboard m_Leaderboard;
	TArray<FSteamID> m_Users;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers")); }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
class STEAMCOREPRO_API FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC : public FOnlineAsyncTaskSteamCorePro
{
public:
	FOnAttachLeaderboardUGC m_OnSteamCallback;
public:
	FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC(const FOnAttachLeaderboardUGC Callback, const FSteamLeaderboard Leaderboard, const FSteamUGCHandle Handle, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, Timeout)
		  , m_OnSteamCallback(Callback)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_UGCHandle(Handle)
	{
	}

	FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC(USteamCoreProAsyncAction* AsyncObject, const FSteamLeaderboard Leaderboard, const FSteamUGCHandle Handle, float Timeout = 10.f)
		: FOnlineAsyncTaskSteamCorePro(k_uAPICallInvalid, AsyncObject, Timeout)
		  , m_CallbackResults()
		  , m_Leaderboard(Leaderboard)
		  , m_UGCHandle(Handle)
	{
		m_OnSteamCallback.BindUFunction(AsyncObject, "HandleCallback");
	}

private:
	FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC() = delete;
protected:
	LeaderboardUGCSet_t m_CallbackResults;
	FSteamLeaderboard m_Leaderboard;
	FSteamUGCHandle m_UGCHandle;
private:
	virtual void Tick() override;
	virtual void TriggerDelegates() override;
	virtual FString ToString() const override { return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC")); }
};
#endif