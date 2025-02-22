/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Achievements/OnlineAchievementsInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE
FOnlineAchievementsSteamCore::FOnlineAchievementsSteamCore(class FOnlineSubsystemSteamCore* InSubsystem)
	: m_SteamSubsystem(InSubsystem)
{
	check(m_SteamSubsystem);

	m_StatsInt = m_SteamSubsystem->GetInternalLeaderboardsInterface();
	check(m_StatsInt);

	m_bHaveConfiguredAchievements = ReadAchievementsFromConfig();
}

bool FOnlineAchievementsSteamCore::ReadAchievementsFromConfig()
{
	LogSteamCoreVerbose("");
	if (m_Achievements.Num() > 0)
	{
		return true;
	}

	FSteamAchievementsConfig Config;
	return Config.ReadAchievements(m_Achievements);
}

void FOnlineAchievementsSteamCore::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");

		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(PlayerId);
	const CSteamID CSID = SteamId;

	if (SteamUser() == nullptr || SteamUser()->GetSteamID() != CSID)
	{
		LogSteamCoreWarn("Cannot report Steam achievements for non-local player %s", *PlayerId.ToString());

		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	const TArray<FOnlineAchievement>* PlayerAch = m_PlayerAchievements.Find(SteamId.AsShared());

	if (PlayerAch == nullptr)
	{
		LogSteamCoreWarn("Steam achievements have not been read for player %s", *PlayerId.ToString());

		WriteObject->WriteState = EOnlineAsyncTaskState::Failed;
		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	const int32 AchNum = PlayerAch->Num();
	for (FStatPropertyArray::TConstIterator It(WriteObject->Properties); It; ++It)
	{
#if UE_VERSION_OLDER_THAN(5,5,0)
		const FString AchievementId = It.Key().ToString();
#else
		const FString AchievementId = It.Key();
#endif
		LogSteamCoreVerbose("WriteObject AchievementId: '%s'", *AchievementId);
		for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
		{
			if ((*PlayerAch)[AchIdx].Id == AchievementId)
			{
#if !UE_BUILD_SHIPPING
				float Value = 0.0f;
				It.Value().GetValue(Value);
				if (Value <= 0.0f)
				{
					LogSteamCoreVerbose("Resetting achievement '%s'", *AchievementId);
					SteamUserStats()->ClearAchievement(TCHAR_TO_UTF8(*AchievementId));
				}
				else
				{
#endif // !UE_BUILD_SHIPPING

					LogSteamCoreVerbose("Setting achievement '%s'", *AchievementId);
					SteamUserStats()->SetAchievement(TCHAR_TO_UTF8(*AchievementId));

#if !UE_BUILD_SHIPPING
				}
#endif // !UE_BUILD_SHIPPING

				break;
			}
		}
	}

	m_StatsInt->WriteAchievementsInternal(SteamId, WriteObject, Delegate);
};

void FOnlineAchievementsSteamCore::OnWriteAchievementsComplete(const FUniqueNetIdSteam& PlayerId, bool bWasSuccessful, FOnlineAchievementsWritePtr& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	check(WriteObject.IsValid());

	if (bWasSuccessful)
	{
		TArray<FOnlineAchievement>* PlayerAch = m_PlayerAchievements.Find(PlayerId.AsShared());
		check(PlayerAch);
		if (PlayerAch != nullptr && WriteObject.IsValid())
		{
			const int32 AchNum = PlayerAch->Num();
			for (FStatPropertyArray::TConstIterator It(WriteObject->Properties); It; ++It)
			{
#if UE_VERSION_OLDER_THAN(5,5,0)
				const FString AchievementId = It.Key().ToString();
#else
				const FString AchievementId = It.Key();
#endif
				for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
				{
					if ((*PlayerAch)[AchIdx].Id == AchievementId)
					{
						(*PlayerAch)[AchIdx].Progress = 100.0;
						TriggerOnAchievementUnlockedDelegates(PlayerId, (*PlayerAch)[AchIdx].Id);
						break;
					}
				}
			}
		}
	}

	Delegate.ExecuteIfBound(PlayerId, bWasSuccessful);
}

void FOnlineAchievementsSteamCore::QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");

		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	m_StatsInt->QueryAchievementsInternal(FUniqueNetIdSteam::Cast(PlayerId), Delegate);
}

void FOnlineAchievementsSteamCore::QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");

		Delegate.ExecuteIfBound(PlayerId, false);
		return;
	}

	m_StatsInt->QueryAchievementsInternal(FUniqueNetIdSteam::Cast(PlayerId), Delegate);
}


void FOnlineAchievementsSteamCore::UpdateAchievementsForUser(const FUniqueNetIdSteam& PlayerId, bool bReadSuccessfully)
{
	LogSteamCoreVerbose("");
	check(m_bHaveConfiguredAchievements);

	ISteamUserStats* SteamUserStatsPtr = SteamUserStats();
	check(SteamUserStatsPtr);

	TArray<FOnlineAchievement> AchievementsForPlayer;
	const int32 AchNum = m_Achievements.Num();

	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		bool bUnlocked;
		uint32 UnlockUnixTime;
		if (!SteamUserStatsPtr->GetAchievementAndUnlockTime(TCHAR_TO_UTF8(*m_Achievements[AchIdx].Id), &bUnlocked, &UnlockUnixTime))
		{
			LogSteamCoreWarn("GetAchievementAndUnlockTime() failed for achievement '%s'", *m_Achievements[AchIdx].Id);
			continue;
		}

		FOnlineAchievementSteam NewAch = m_Achievements[AchIdx];
		NewAch.m_bReadFromSteam = true;
		NewAch.Progress = bUnlocked ? 100.0 : 0.0;
		NewAch.UnlockTime = FDateTime::FromUnixTimestamp(UnlockUnixTime);

		NewAch.Title = FText::FromString(UTF8_TO_TCHAR(SteamUserStatsPtr->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*m_Achievements[AchIdx].Id), "name")));
		NewAch.LockedDesc = FText::FromString(UTF8_TO_TCHAR(SteamUserStatsPtr->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*m_Achievements[AchIdx].Id), "desc")));
		NewAch.UnlockedDesc = NewAch.LockedDesc;

		NewAch.bIsHidden = FCString::Atoi(UTF8_TO_TCHAR(SteamUserStatsPtr->GetAchievementDisplayAttribute(TCHAR_TO_UTF8(*m_Achievements[AchIdx].Id), "hidden"))) != 0;

		LogSteamCoreVerbose("Read achievement %d: %s", AchIdx, *NewAch.ToDebugString());
		AchievementsForPlayer.Add(NewAch);

		m_AchievementDescriptions.Add(NewAch.Id, NewAch);
	}

	m_PlayerAchievements.Add(PlayerId.AsShared(), AchievementsForPlayer);
}

EOnlineCachedResult::Type FOnlineAchievementsSteamCore::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement)
{
	LogSteamCoreVeryVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");
		return EOnlineCachedResult::NotFound;
	}

	const TArray<FOnlineAchievement>* PlayerAch = m_PlayerAchievements.Find(PlayerId.AsShared());
	if (PlayerAch == nullptr)
	{
		LogSteamCoreWarn("Steam achievements have not been read for player %s", *PlayerId.ToString());
		return EOnlineCachedResult::NotFound;
	}

	const int32 AchNum = PlayerAch->Num();
	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		if ((*PlayerAch)[AchIdx].Id == AchievementId)
		{
			OutAchievement = (*PlayerAch)[AchIdx];
			return EOnlineCachedResult::Success;
		}
	}

	LogSteamCoreWarn("Could not find Steam achievement '%s' for player %s", *AchievementId, *PlayerId.ToString());
	return EOnlineCachedResult::NotFound;
};

EOnlineCachedResult::Type FOnlineAchievementsSteamCore::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements)
{
	LogSteamCoreVeryVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");
		return EOnlineCachedResult::NotFound;
	}

	const TArray<FOnlineAchievement>* PlayerAch = m_PlayerAchievements.Find(PlayerId.AsShared());
	if (PlayerAch == nullptr)
	{
		LogSteamCoreWarn("Steam achievements have not been read for player %s", *PlayerId.ToString());
		return EOnlineCachedResult::NotFound;
	}

	OutAchievements = *PlayerAch;
	return EOnlineCachedResult::Success;
};

EOnlineCachedResult::Type FOnlineAchievementsSteamCore::GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
{
	LogSteamCoreVeryVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");
		return EOnlineCachedResult::NotFound;
	}

	if (m_AchievementDescriptions.Num() == 0)
	{
		LogSteamCoreWarn("Descriptions have not been read");
		return EOnlineCachedResult::NotFound;
	}

	const FOnlineAchievementDesc* AchDesc = m_AchievementDescriptions.Find(AchievementId);
	if (AchDesc == nullptr)
	{
		LogSteamCoreWarn("Achievement '%s' does not have a description", *AchievementId);
		return EOnlineCachedResult::NotFound;
	}

	OutAchievementDesc = *AchDesc;
	return EOnlineCachedResult::Success;
};

#if !UE_BUILD_SHIPPING
bool FOnlineAchievementsSteamCore::ResetAchievements(const FUniqueNetId& PlayerId)
{
	LogSteamCoreVerbose("");
	if (!m_bHaveConfiguredAchievements)
	{
		LogSteamCoreWarn("Steam achievements have not been configured in .ini");
		return false;
	}

	const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(PlayerId);
	if (SteamUser() == nullptr || SteamUser()->GetSteamID() != SteamId)
	{
		LogSteamCoreWarn("Cannot clear Steam achievements for non-local player %s", *PlayerId.ToString());
		return false;
	}

	const TArray<FOnlineAchievement>* PlayerAch = m_PlayerAchievements.Find(SteamId.AsShared());
	if (PlayerAch == nullptr)
	{
		LogSteamCoreWarn("Steam achievements have not been read for player %s", *PlayerId.ToString());
		return false;
	}

	const int32 AchNum = PlayerAch->Num();
	for (int32 AchIdx = 0; AchIdx < AchNum; ++AchIdx)
	{
		SteamUserStats()->ClearAchievement(TCHAR_TO_UTF8(*(*PlayerAch)[ AchIdx ].Id));
	}

	m_StatsInt->FlushLeaderboards(FName(TEXT("UNUSED")));
	return true;
};
#endif // !UE_BUILD_SHIPPING
#endif