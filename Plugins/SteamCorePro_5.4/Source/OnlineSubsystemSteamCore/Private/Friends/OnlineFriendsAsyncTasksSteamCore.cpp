/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "OnlineFriendsAsyncTasksSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

void FOnlineAsyncTaskSteamCoreReadFriendsList::Finalize()
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	const FOnlineSubsystemSteamCore* SteamSubsystem = m_FriendsPtr->m_SteamSubsystem;
	ISteamFriends* SteamFriendsPtr = m_FriendsPtr->m_SteamFriendsPtr;
	FOnlineFriendsSteamCore::FSteamFriendsList& FriendsList = m_FriendsPtr->m_FriendsLists.FindOrAdd(m_LocalUserNum);

	const int32 NumFriends = SteamFriendsPtr->GetFriendCount(k_EFriendFlagImmediate);
	FriendsList.m_Friends.Empty(NumFriends);
	for (int32 Index = 0; Index < NumFriends; Index++)
	{
		const CSteamID SteamPlayerId = SteamFriendsPtr->GetFriendByIndex(Index, k_EFriendFlagImmediate);
		const FString NickName(UTF8_TO_TCHAR(SteamFriendsPtr->GetFriendPersonaName(SteamPlayerId)));

		bool bInASession;
		FriendGameInfo_t FriendGameInfo;
		const bool bIsPlayingAGame = SteamFriendsPtr->GetFriendGamePlayed(SteamPlayerId, &FriendGameInfo);
		const bool bIsOnline = (SteamFriendsPtr->GetFriendPersonaState(SteamPlayerId) >= k_EPersonaStateOnline);
		const bool bIsPlayingThisGame = (FriendGameInfo.m_gameID.AppID() == SteamSubsystem->GetSteamAppId());
		const bool bHasConnectInformation = (SteamFriendsPtr->GetFriendRichPresence(SteamPlayerId, "connect") != nullptr);
		FString JoinablePresenceString = UTF8_TO_TCHAR(SteamFriendsPtr->GetFriendRichPresence(SteamPlayerId, "Joinable"));

		if (!JoinablePresenceString.IsEmpty())
		{
			bInASession = (JoinablePresenceString == TEXT("true"));
		}
		else
		{
			bInASession = bIsPlayingThisGame && bHasConnectInformation;
		}

		if (NickName.Len() > 0 && CanAddUserToList(bIsOnline, bIsPlayingThisGame, bInASession))
		{
			TSharedRef<FOnlineFriendSteamCore> Friend(new FOnlineFriendSteamCore(SteamPlayerId));
			FriendsList.m_Friends.Add(Friend);

			Friend->m_AccountData.Add(TEXT("nickname"), NickName);
			Friend->m_Presence.Status.StatusStr = UTF8_TO_TCHAR(SteamFriendsPtr->GetFriendRichPresence(SteamPlayerId,"status"));
			Friend->m_Presence.bIsJoinable = bInASession;
			Friend->m_Presence.bIsOnline = bIsOnline;
			Friend->m_Presence.bIsPlaying = bIsPlayingAGame;
			Friend->m_Presence.bIsPlayingThisGame = bIsPlayingThisGame;

			switch (SteamFriendsPtr->GetFriendPersonaState(SteamPlayerId))
			{
			case k_EPersonaStateOffline:
				Friend->m_Presence.Status.State = EOnlinePresenceState::Offline;
				break;
			case k_EPersonaStateBusy:
				Friend->m_Presence.Status.State = EOnlinePresenceState::DoNotDisturb;
				break;
			case k_EPersonaStateAway:
				Friend->m_Presence.Status.State = EOnlinePresenceState::Away;
				break;
			case k_EPersonaStateSnooze:
				Friend->m_Presence.Status.State = EOnlinePresenceState::ExtendedAway;
				break;
			default:
				Friend->m_Presence.Status.State = EOnlinePresenceState::Online;
				break;
			}

			FString VoicePresenceString = UTF8_TO_TCHAR(SteamFriendsPtr->GetFriendRichPresence(SteamPlayerId,"HasVoice"));
			Friend->m_Presence.bHasVoiceSupport = VoicePresenceString == TEXT("true");
		}
	}
#endif
}

void FOnlineAsyncTaskSteamCoreReadFriendsList::TriggerDelegates(void)
{
	LogSteamCoreVerbose("");
	FOnlineAsyncTask::TriggerDelegates();

	m_Delegate.ExecuteIfBound(m_LocalUserNum, true, EFriendsLists::ToString(m_FriendsListFilter), FString());
}
