/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Presence/OnlinePresenceInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE

FOnlinePresenceSteamCore::FOnlinePresenceSteamCore(class FOnlineSubsystemSteamCore* InSubsystem) :
	m_SteamFriendsPtr(SteamFriends()),
	m_SteamSubsystem(InSubsystem)
	
{
}

FOnlinePresenceSteamCore::FOnlinePresenceSteamCore() : 
	m_SteamFriendsPtr(nullptr),
	m_SteamSubsystem(nullptr)
{
}

void FOnlinePresenceSteamCore::SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (m_SteamFriendsPtr == nullptr || m_SteamSubsystem == nullptr)
	{
		LogSteamCoreWarn("Steam friends is null, cannot set presence!");
		Delegate.ExecuteIfBound(User, false);
		return;
	}

	m_SteamFriendsPtr->ClearRichPresence();
	if (Status.Properties.Num() > k_cchMaxRichPresenceKeys)
	{
		// This doesn't account for the rich presence status and the connection information.
		LogSteamCoreError("Number of presence properties (%d) exceeds maximum keys allowed!", Status.Properties.Num());
		Delegate.ExecuteIfBound(User, false);
		return;
	}

	if (!m_SteamFriendsPtr->SetRichPresence(TCHAR_TO_UTF8(*DefaultSteamPresenceKey), TCHAR_TO_UTF8(*Status.StatusStr)))
	{
		if (Status.StatusStr.Len() >= k_cchMaxRichPresenceValueLength)
		{
			LogSteamCoreWarn("Cannot push rich presence status to steam, string is too long (%d)", Status.StatusStr.Len());
		}
		else
		{
			LogSteamCoreWarn("An unknown error occurred when trying to push rich presence status to steam!");
		}
	}

	const FOnlineSessionSteamCorePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionSteamCore>(m_SteamSubsystem->GetSessionInterface());
	if (SessionInterface.IsValid())
	{
		const FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (CurrentSession != nullptr && CurrentSession->SessionSettings.bAllowJoinViaPresence)
		{
			const FString SteamConnectString = SessionInterface->GetSteamConnectionString(NAME_GameSession);
			if (!SteamConnectString.IsEmpty() && !m_SteamFriendsPtr->SetRichPresence(TCHAR_TO_UTF8(*DefaultSteamConnectionKey), TCHAR_TO_UTF8(*SteamConnectString)))
			{
				LogSteamCoreWarn("Could not push the connection information to Steam");
			}
		}
	}

	for (FPresenceProperties::TConstIterator Itr(Status.Properties); Itr; ++Itr)
	{
		const FString& Key = Itr.Key();
		const FString& Value = Itr.Value().ToString();
		if (!m_SteamFriendsPtr->SetRichPresence(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value)))
		{
			if (Key.Len() >= k_cchMaxRichPresenceKeyLength || Key.IsEmpty())
			{
				LogSteamCoreWarn("Steam presence key %s is either empty or over the max length of a key", *Key);
			}
			else if(Value.Len() >= k_cchMaxRichPresenceValueLength)
			{
				LogSteamCoreWarn("Steam presence value for key %s (%d) is over the max size allowed", *Key, Value.Len());
			}
			else
			{
				LogSteamCoreWarn("Could not push presence key %s to steam!", *Key);
			}
		}
	}
	
	QueryPresence(User, Delegate);
}

void FOnlinePresenceSteamCore::QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	if (m_SteamFriendsPtr == nullptr || m_SteamSubsystem == nullptr)
	{
		LogSteamCoreWarn("Steam friends is null, cannot fetch presence!");
		Delegate.ExecuteIfBound(User, false);
		return;
	}

	const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(User);
	if (!SteamId.IsValid())
	{
		LogSteamCoreWarn("User id %s is not valid, cannot query presence", *SteamId.ToString());
		Delegate.ExecuteIfBound(User, false);
		return;
	}

	const TSharedRef<FOnlineUserPresenceSteamCore>* FoundEntry = m_CachedPresence.Find(SteamId.AsShared());
	
	if (FoundEntry == nullptr)
	{
		const TSharedRef<FOnlineUserPresenceSteamCore> Presence(new FOnlineUserPresenceSteamCore());
		FoundEntry = &m_CachedPresence.Add(SteamId.AsShared(), Presence);
	}
	
	if (m_SteamFriendsPtr->GetFriendRelationship(SteamId) != k_EFriendRelationshipFriend && !m_SteamSubsystem->IsLocalPlayer(User))
	{
		m_DelayedPresenceDelegates.Add(SteamId.AsShared(), MakeShared<const FOnPresenceTaskCompleteDelegate>(Delegate));
		m_SteamFriendsPtr->RequestFriendRichPresence(SteamId);
		return;
	}

	FoundEntry->Get().Update(SteamId);
	Delegate.ExecuteIfBound(User, true);
}

void FOnlinePresenceSteamCore::UpdatePresenceForUser(const FUniqueNetId& User)
{
	LogSteamCoreVerbose("");
	if (m_SteamFriendsPtr == nullptr || m_SteamSubsystem == nullptr || m_SteamSubsystem->IsLocalPlayer(User))
	{
		return;
	}

	const FUniqueNetIdSteam& SteamId = FUniqueNetIdSteam::Cast(User);
	const TSharedRef<FOnlineUserPresenceSteamCore>* FoundEntry = m_CachedPresence.Find(SteamId.AsShared());

	if (FoundEntry == nullptr)
	{
		const TSharedRef<FOnlineUserPresenceSteamCore> Presence(new FOnlineUserPresenceSteamCore());
		FoundEntry = &m_CachedPresence.Add(SteamId.AsShared(), Presence);
	}

	FoundEntry->Get().Update(SteamId);
	const TSharedRef<const FOnPresenceTaskCompleteDelegate>* DelayedDelegate = m_DelayedPresenceDelegates.Find(SteamId.AsShared());
	
	if (DelayedDelegate != nullptr)
	{
		DelayedDelegate->Get().ExecuteIfBound(User, true);
		m_DelayedPresenceDelegates.Remove(SteamId.AsShared());
	}
	else
	{
		TArray<TSharedRef<FOnlineUserPresence> > PresenceArray;
		PresenceArray.Add(*FoundEntry);

		TriggerOnPresenceArrayUpdatedDelegates(User, PresenceArray);
		TriggerOnPresenceReceivedDelegates(User, *FoundEntry);
	}
}

EOnlineCachedResult::Type FOnlinePresenceSteamCore::GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence)
{
	LogSteamCoreVerbose("");
	const TSharedRef<FOnlineUserPresenceSteamCore>* Found = m_CachedPresence.Find(User.AsShared());
	if (Found == nullptr)
	{
		return EOnlineCachedResult::NotFound;
	}

	OutPresence = MakeShared<FOnlineUserPresence>(Found->Get());
	return EOnlineCachedResult::Success;
}

EOnlineCachedResult::Type FOnlinePresenceSteamCore::GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence)
{
	LogSteamCoreVerbose("");
	if (SteamUtils() != nullptr && SteamUtils()->GetAppID() == FCString::Atoi(*AppId))
	{
		return GetCachedPresence(User, OutPresence);
	}

	return EOnlineCachedResult::NotFound;
}
#endif