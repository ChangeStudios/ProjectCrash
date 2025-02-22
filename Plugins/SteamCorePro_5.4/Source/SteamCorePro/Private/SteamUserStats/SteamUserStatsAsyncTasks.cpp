/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUserStats
*/

#include "SteamUserStats/SteamUserStatsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

#if WITH_STEAMCORE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();

	if (bIsComplete)
	{
		return;
	}
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->FindLeaderboard(TCHAR_TO_UTF8(*m_LeaderboardName));
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_bLeaderboardFound) ? true : false) && ((m_CallbackResults.m_hSteamLeaderboard > 0 ? true : false));
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsFindLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->DownloadLeaderboardEntries(m_Leaderboard, static_cast<ELeaderboardDataRequest>(m_DataRequest), m_RangeStart, m_RangeEnd);
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_hSteamLeaderboard > 0 ? true : false));
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntries::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->RequestGlobalStats(m_HistoryDays);
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_eResult == k_EResultOK ? true : false));
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalStats::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->RequestGlobalAchievementPercentages();
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_eResult == k_EResultOK ? true : false));
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestGlobalAchievementPercentages
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->GetNumberOfCurrentPlayers();
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_bSuccess > 0 ? true : false));
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsGetNumberOfCurrentPlayers::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->UploadLeaderboardScore(m_Leaderboard, static_cast<ELeaderboardUploadScoreMethod>(m_UploadScoreMethod), m_Score, m_ScoreDetails.GetData(), m_ScoreDetails.Num());
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_bSuccess > 0) ? true : false);
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsUploadLeaderboardScore::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard::Tick()
{
	bWasSuccessful = false;

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->FindOrCreateLeaderboard(TCHAR_TO_UTF8(*m_LeaderboardName), static_cast<ELeaderboardSortMethod>(m_SortMethod), static_cast<ELeaderboardDisplayType>(m_DisplayType));
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_bLeaderboardFound) ? true : false);
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsFindOrCreateLeaderboard::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->RequestUserStats(m_SteamID);
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_eResult == k_EResultOK) ? true : false);
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsRequestUserStats::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->DownloadLeaderboardEntriesForUsers(m_Leaderboard, reinterpret_cast<CSteamID*>(m_Users.GetData()), m_Users.Num());
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_hSteamLeaderboard > 0) ? true : false);
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsDownloadLeaderboardEntriesForUsers::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC::Tick()
{
	FOnlineAsyncTaskSteamCorePro::Tick();

	ISteamUtils* SteamUtilsPtr = IsRunningDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	checkf(SteamUtilsPtr, TEXT("Steam API not found, make sure your Steam Client is running and that the Steam API was loaded."));

	if (bIsComplete)
	{
		return;
	}

	if (SteamUtilsPtr)
	{
		if (!bInit)
		{
			m_CallbackHandle = SteamUserStats()->AttachLeaderboardUGC(m_Leaderboard, m_UGCHandle);
			bInit = true;
		}

		if (m_CallbackHandle != k_uAPICallInvalid)
		{
			bool bFailedCall = false;

			bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;

			if (bIsComplete)
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) && (!bFailedResult ? true : false) && ((m_CallbackResults.m_hSteamLeaderboard > 0) ? true : false);
			}
		}
		else
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
	else
	{
		LogSteamCoreError("SteamUtilsPtr was nullptr");
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreProUserStatsAttachLeaderboardUGC::TriggerDelegates()
{
	LogSteamCoreVerbose("WasSuccessful: %d", WasSuccessful());

	m_OnSteamCallback.ExecuteIfBound(m_CallbackResults, bWasSuccessful);
}
#endif