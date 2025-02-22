/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystemSteamCore.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAchievementsSteamCore : public IOnlineAchievements
{
private:
	struct FOnlineAchievementSteam : public FOnlineAchievement, public FOnlineAchievementDesc
	{
		bool m_bReadFromSteam;

		FString ToDebugString() const
		{
			return FString::Printf(TEXT("Achievement:{%s},  Description:{%s}, bReadFromSteam=%s"), *FOnlineAchievement::ToDebugString(), *FOnlineAchievementDesc::ToDebugString(), m_bReadFromSteam ? TEXT("true") : TEXT("false"));
		}
	};

	struct FSteamAchievementsConfig
	{
		FSteamAchievementsConfig()
			: m_IniName(GEngineIni)
			  , m_SectionName(TEXT("OnlineSubsystemSteamCore"))
		{
		}

		FString GetKey(const FString& KeyName) const
		{
			FString Result;
			if (!GConfig->GetString(*m_SectionName, *KeyName, Result, m_IniName))
			{
				return TEXT("");
			}
			return Result;
		}

		bool ReadAchievements(TArray<FOnlineAchievementSteam>& OutArray) const
		{
			OutArray.Empty();
			int NumAchievements = 0;

			for (;; ++NumAchievements)
			{
				FString Id = GetKey(FString::Printf(TEXT("Achievement_%d_Id"), NumAchievements));
				if (Id.IsEmpty())
				{
					break;
				}

				FOnlineAchievementSteam NewAch;
				NewAch.Id = Id;
				NewAch.Progress = 0.0;
				NewAch.m_bReadFromSteam = false;

				OutArray.Add(NewAch);
			}

			return NumAchievements > 0;
		}

		FString m_IniName;
		FString m_SectionName;
	};

	FOnlineAchievementsSteamCore()
		: m_SteamSubsystem(nullptr), m_StatsInt(nullptr), m_bHaveConfiguredAchievements(false)
	{
	};

private:
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	FOnlineLeaderboardsSteamCore* m_StatsInt;
	TUniqueNetIdMap<TArray<FOnlineAchievement>> m_PlayerAchievements;
	TMap<FString, FOnlineAchievementDesc> m_AchievementDescriptions;
	TArray<FOnlineAchievementSteam> m_Achievements;
	bool m_bHaveConfiguredAchievements;

private:
	bool ReadAchievementsFromConfig();
PACKAGE_SCOPE:
	void UpdateAchievementsForUser(const FUniqueNetIdSteam& PlayerId, bool bReadSuccessfully);
	void OnWriteAchievementsComplete(const FUniqueNetIdSteam& PlayerId, bool bWasSuccessful, FOnlineAchievementsWritePtr& WriteObject, const FOnAchievementsWrittenDelegate& Delegate);

public:
	FOnlineAchievementsSteamCore(FOnlineSubsystemSteamCore* InSubsystem);

	virtual ~FOnlineAchievementsSteamCore() override
	{
	}

	//~ Begin IOnlineAchievements Interface
	virtual void WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate = FOnAchievementsWrittenDelegate()) override;
	virtual void QueryAchievements(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate = FOnQueryAchievementsCompleteDelegate()) override;
	virtual void QueryAchievementDescriptions(const FUniqueNetId& PlayerId, const FOnQueryAchievementsCompleteDelegate& Delegate = FOnQueryAchievementsCompleteDelegate()) override;
	virtual EOnlineCachedResult::Type GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement) override;
	virtual EOnlineCachedResult::Type GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements) override;
	virtual EOnlineCachedResult::Type GetCachedAchievementDescription(const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc) override;
#if !UE_BUILD_SHIPPING
	virtual bool ResetAchievements(const FUniqueNetId& PlayerId) override;
#endif // !UE_BUILD_SHIPPING
	//~ End IOnlineAchievements Interface
};
#else
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAchievementsSteamCore
{
};
#endif