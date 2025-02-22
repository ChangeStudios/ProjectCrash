/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Presence/OnlinePresenceTypesSteamCore.h"

#if WITH_STEAMCORE
void FOnlineUserPresenceSteamCore::Update(const FUniqueNetIdSteam& FriendId)
{
	LogSteamCoreVerbose("");
	Reset();
	ISteamFriends* SteamFriendPtr = SteamFriends();
	if (SteamFriendPtr == nullptr || SteamUtils() == nullptr)
	{
		return;
	}

	FriendGameInfo_t FriendInfo;
	m_bIsAFriend = (SteamFriendPtr->GetFriendRelationship(FriendId) == k_EFriendRelationshipFriend);
	bIsOnline = true;

	switch (SteamFriendPtr->GetFriendPersonaState(FriendId))
	{
	default:
	case k_EPersonaStateOffline:
		bIsOnline = false;
		Status.State = EOnlinePresenceState::Offline;
		break;
	case k_EPersonaStateBusy:
		Status.State = EOnlinePresenceState::DoNotDisturb;
		break;
	case k_EPersonaStateAway:
		Status.State = EOnlinePresenceState::Away;
		break;
	case k_EPersonaStateSnooze:
		Status.State = EOnlinePresenceState::ExtendedAway;
		break;
	case k_EPersonaStateOnline:
	case k_EPersonaStateLookingToTrade:
	case k_EPersonaStateLookingToPlay:
		Status.State = EOnlinePresenceState::Online;
		break;
	}

	if (SteamFriendPtr->GetFriendGamePlayed(FriendId, &FriendInfo))
	{
		bIsPlaying = true;
		SessionId = FUniqueNetIdSteam::Create(FriendInfo.m_steamIDLobby); 
		bIsPlayingThisGame = (FriendInfo.m_gameID.AppID() == SteamUtils()->GetAppID());
	}

	for (int32 RPIdx = 0; RPIdx < SteamFriendPtr->GetFriendRichPresenceKeyCount(FriendId); ++RPIdx)
	{
		FString Key = SteamFriendPtr->GetFriendRichPresenceKeyByIndex(FriendId, RPIdx);
		FString Value = SteamFriendPtr->GetFriendRichPresence(FriendId, TCHAR_TO_UTF8(*Key));

		if (Key == DefaultSteamConnectionKey)
		{
			bIsJoinable = true;
		}

		if (Key == DefaultSteamPresenceKey)
		{
			Status.StatusStr = Value;
			continue;
		}

		FVariantData PropertyData;
		PropertyData.SetValue(Value);
		Status.Properties.Add(Key, PropertyData);
	}
}
#endif
