/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamFriends
*/

#include "SteamFriends/SteamFriends.h"
#include "SteamFriends/SteamFriendsAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

UTexture2D* USteamProFriends::GetAvatar(uint8 Size, FSteamID SteamUserID)
{
	UTexture2D* Result = nullptr;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		int Data = 0;

		switch (Size)
		{
		case 0:
			Data = SteamFriends()->GetSmallFriendAvatar(SteamUserID);
			break;
		case 1:
			Data = SteamFriends()->GetMediumFriendAvatar(SteamUserID);
			break;
		case 2:
			Data = SteamFriends()->GetLargeFriendAvatar(SteamUserID);
			break;
		}

		Result = GetSteamTexture(Data);
	}
	// size: 0=small, 1=medium, 2=large
#endif

	return Result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

USteamProFriends::USteamProFriends()
{
#if WITH_STEAMCORE
	OnAvatarImageLoadedCallback.Register(this, &USteamProFriends::OnAvatarImageLoaded);
	OnFriendRichPresenceUpdateCallback.Register(this, &USteamProFriends::OnFriendRichPresenceUpdate);
	OnGameConnectedChatJoinCallback.Register(this, &USteamProFriends::OnGameConnectedChatJoin);
	OnGameConnectedChatLeaveCallback.Register(this, &USteamProFriends::OnGameConnectedChatLeave);
	OnGameConnectedFriendChatMsgCallback.Register(this, &USteamProFriends::OnGameConnectedFriendChatMsg);
	OnGameConnectedClanChatMsgCallback.Register(this, &USteamProFriends::OnGameConnectedClanChatMsg);
	OnGameLobbyJoinRequestedCallback.Register(this, &USteamProFriends::OnGameLobbyJoinRequested);
	OnGameOverlayActivatedCallback.Register(this, &USteamProFriends::OnGameOverlayActivated);
	OnGameRichPresenceJoinRequestedCallback.Register(this, &USteamProFriends::OnGameRichPresenceJoinRequested);
	OnGameServerChangeRequestedCallback.Register(this, &USteamProFriends::OnGameServerChangeRequested);
	OnPersonaStateChangeCallback.Register(this, &USteamProFriends::OnPersonaStateChange);
	OnSetPersonaNameResponseCallback.Register(this, &USteamProFriends::OnSetPersonaNameResponse);
	OnJoinClanChatRoomCompletionResultCallback.Register(this, &USteamProFriends::OnJoinClanChatRoomCompletionResult);
	OnClanOfficerListResponseCallback.Register(this, &USteamProFriends::OnClanOfficerListResponse);
	OnDownloadClanActivityCountsResultCallback.Register(this, &USteamProFriends::OnDownloadClanActivityCountsResult);
	OnEquippedProfileItemsChangedCallback.Register(this, &USteamProFriends::OnEquippedProfileItemsChanged);

	if (IsRunningDedicatedServer())
	{
		OnAvatarImageLoadedCallback.SetGameserverFlag();
		OnFriendRichPresenceUpdateCallback.SetGameserverFlag();
		OnGameConnectedChatJoinCallback.SetGameserverFlag();
		OnGameConnectedChatLeaveCallback.SetGameserverFlag();
		OnGameConnectedFriendChatMsgCallback.SetGameserverFlag();
		OnGameConnectedClanChatMsgCallback.SetGameserverFlag();
		OnGameLobbyJoinRequestedCallback.SetGameserverFlag();
		OnGameOverlayActivatedCallback.SetGameserverFlag();
		OnGameRichPresenceJoinRequestedCallback.SetGameserverFlag();
		OnGameServerChangeRequestedCallback.SetGameserverFlag();
		OnPersonaStateChangeCallback.SetGameserverFlag();
		OnSetPersonaNameResponseCallback.SetGameserverFlag();
		OnJoinClanChatRoomCompletionResultCallback.SetGameserverFlag();
		OnClanOfficerListResponseCallback.SetGameserverFlag();
		OnDownloadClanActivityCountsResultCallback.SetGameserverFlag();
		OnEquippedProfileItemsChangedCallback.SetGameserverFlag();
	}
#endif
}

USteamProFriends::~USteamProFriends()
{
#if WITH_STEAMCORE
	OnAvatarImageLoadedCallback.Unregister();
	OnFriendRichPresenceUpdateCallback.Unregister();
	OnGameConnectedChatJoinCallback.Unregister();
	OnGameConnectedChatLeaveCallback.Unregister();
	OnGameConnectedFriendChatMsgCallback.Unregister();
	OnGameConnectedClanChatMsgCallback.Unregister();
	OnGameLobbyJoinRequestedCallback.Unregister();
	OnGameOverlayActivatedCallback.Unregister();
	OnGameRichPresenceJoinRequestedCallback.Unregister();
	OnGameServerChangeRequestedCallback.Unregister();
	OnPersonaStateChangeCallback.Unregister();
	OnSetPersonaNameResponseCallback.Unregister();
	OnJoinClanChatRoomCompletionResultCallback.Unregister();
	OnClanOfficerListResponseCallback.Unregister();
	OnDownloadClanActivityCountsResultCallback.Unregister();
	OnEquippedProfileItemsChangedCallback.Unregister();
#endif
}

USteamProFriends* USteamProFriends::GetSteamFriends()
{
#if WITH_STEAMCORE
	return SteamFriends() ? ThisClass::StaticClass()->GetDefaultObject<USteamProFriends>() : nullptr;
#else
	return nullptr;
#endif
}

void USteamProFriends::ActivateGameOverlay(FString Dialog)
{
	LogSteamCoreVerbose("");
#if WITH_STEAMCORE

	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlay(TCHAR_TO_UTF8(*Dialog));
	}
#endif
}

void USteamProFriends::ActivateGameOverlayInvitedialog(FSteamID SteamIDLobby)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlayInviteDialog(SteamIDLobby);
	}
#endif
}

void USteamProFriends::ActivateGameOverlayToStore(int32 AppID, ESteamOverlayToStoreFlag Flag)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlayToStore(AppID, static_cast<EOverlayToStoreFlag>(Flag));
	}
#endif
}

void USteamProFriends::ActivateGameOverlayToUser(FString Dialog, FSteamID SteamID)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlayToUser(TCHAR_TO_UTF8(*Dialog), SteamID);
	}
#endif
}

void USteamProFriends::ActivateGameOverlayToWebPage(FString URL, ESteamActivateGameOverlayToWebPageMode Mode)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlayToWebPage(TCHAR_TO_UTF8(*URL), static_cast<EActivateGameOverlayToWebPageMode>(Mode));
	}
#endif
}

void USteamProFriends::ClearRichPresence()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ClearRichPresence();
	}
#endif
}

bool USteamProFriends::CloseClanChatWindowInSteam(FSteamID SteamIDClanChat)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->CloseClanChatWindowInSteam(SteamIDClanChat);
	}
#endif

	return bResult;
}

void USteamProFriends::DownloadClanActivityCounts(const FOnDownloadClanActivityCounts& Callback, TArray<FSteamID> SteamIDClans)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts* Task = new FOnlineAsyncTaskSteamCoreProFriendsDownloadClanActivityCounts(Callback, SteamIDClans);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProFriends::EnumerateFollowingList(const FOnEnumerateFollowingList& Callback, int32 StartIndex)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList* Task = new FOnlineAsyncTaskSteamCoreProFriendsEnumerateFollowingList(Callback, StartIndex);
		QueueAsyncTask(Task);
	}
#endif
}

FSteamID USteamProFriends::GetChatMemberByIndex(FSteamID SteamIDClan, int32 User)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetChatMemberByIndex(SteamIDClan, User);
	}
#endif

	return Result;
}

bool USteamProFriends::GetClanActivityCounts(FSteamID SteamIDClan, int32& OutOnline, int32& InGame, int32& OutChatting)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutOnline = 0;
	InGame = 0;
	OutChatting = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->GetClanActivityCounts(SteamIDClan, &OutOnline, &InGame, &OutChatting);
	}
#endif

	return bResult;
}

FSteamID USteamProFriends::GetClanByIndex(int32 Clan)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanByIndex(Clan);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetClanChatMemberCount(FSteamID SteamIDClan)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanChatMemberCount(SteamIDClan);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetClanChatMessage(FSteamID SteamIDClanChat, int32 MessageID, FString& OutText, ESteamChatEntryType& OutChatEntryType, FSteamID& OutSteamIdChatter)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutText.Empty();

#if WITH_STEAMCORE
	CSteamID SteamIdChatter;
	EChatEntryType ChatEntryType = k_EChatEntryTypeInvalid;
	
	if (SteamFriends())
	{
		TArray<char> DataArray;
		DataArray.SetNum(1024);

		Result = SteamFriends()->GetClanChatMessage(SteamIDClanChat, MessageID, DataArray.GetData(), DataArray.Num(), &ChatEntryType, &SteamIdChatter);
		OutText = UTF8_TO_TCHAR(DataArray.GetData());
	}

	OutChatEntryType = static_cast<ESteamChatEntryType>(ChatEntryType);
	OutSteamIdChatter = SteamIdChatter;
#endif

	return Result;
}

int32 USteamProFriends::GetClanCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanCount();
	}
#endif

	return Result;
}

FString USteamProFriends::GetClanName(FSteamID SteamIDClan)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetClanName(SteamIDClan));
	}
#endif

	return Result;
}

FSteamID USteamProFriends::GetClanOfficerByIndex(FSteamID SteamIDClan, int32 Officer)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanOfficerByIndex(SteamIDClan, Officer);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetClanOfficerCount(FSteamID SteamIDClan)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanOfficerCount(SteamIDClan);
	}
#endif

	return Result;
}

FSteamID USteamProFriends::GetClanOwner(FSteamID SteamIDClan)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetClanOwner(SteamIDClan);
	}
#endif

	return Result;
}

FString USteamProFriends::GetClanTag(FSteamID SteamIDClan)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetClanTag(SteamIDClan));
	}
#endif

	return Result;
}

FSteamID USteamProFriends::GetCoplayFriend(int32 CoplayFriend)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetCoplayFriend(CoplayFriend);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetCoplayFriendCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetCoplayFriendCount();
	}
#endif

	return Result;
}

void USteamProFriends::GetFollowerCount(const FOnGetFollowerCount& Callback, FSteamID SteamID)
{
	LogSteamCoreVeryVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount* Task = new FOnlineAsyncTaskSteamCoreProFriendsGetFollowerCount(Callback, SteamID);
		QueueAsyncTask(Task);
	}
#endif
}

FSteamID USteamProFriends::GetFriendByIndex(int32 Ifriend, TArray<ESteamFriendFlags> Flags)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		int Data = 0;

		for (int i = 0; i < Flags.Num(); i++)
		{
			Data |= *Flags[i];
		}

		Result = SteamFriends()->GetFriendByIndex(Ifriend, Data);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendCoplayGame(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendCoplayGame(SteamIDFriend);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendCoplayTime(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendCoplayTime(SteamIDFriend);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendCount(TArray<ESteamFriendFlags> Flags)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		int Data = 0;

		for (int i = 0; i < Flags.Num(); i++)
		{
			Data |= *Flags[i];
		}

		Result = SteamFriends()->GetFriendCount(Data);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendCountFromSource(FSteamID SteamIDSource)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendCountFromSource(SteamIDSource);
	}
#endif

	return Result;
}

FSteamID USteamProFriends::GetFriendFromSourceByIndex(FSteamID SteamIDSource, int32 Ifriend)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendFromSourceByIndex(SteamIDSource, Ifriend);
	}
#endif

	return Result;
}

bool USteamProFriends::GetFriendGamePlayed(FSteamID SteamIDFriend, FSteamGameID& OutGameId, FString& OutGameIp, int32& OutConnectionPort, int32& OutQueryPort, FSteamID& OutSteamLobby)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutGameId = FSteamGameID();
	OutGameIp.Empty();
	OutConnectionPort = 0;
	OutQueryPort = 0;
	OutSteamLobby = FSteamID();

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FriendGameInfo_t FriendData;

		bResult = SteamFriends()->GetFriendGamePlayed(SteamIDFriend, &FriendData);

		if (bResult)
		{
			OutGameId = FriendData.m_gameID;
			OutGameIp = FIPv4Address(FriendData.m_unGameIP).ToString();
			OutConnectionPort = FriendData.m_usGamePort;
			OutQueryPort = FriendData.m_usQueryPort;
			OutSteamLobby = FriendData.m_steamIDLobby;
		}
	}
#endif

	return bResult;
}

int32 USteamProFriends::GetFriendMessage(FSteamID SteamIDFriend, int32 MessageID, FString& OutText, ESteamChatEntryType& OutChatEntryType)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutText.Empty();
#if WITH_STEAMCORE
	EChatEntryType ChatEntryType = k_EChatEntryTypeChatMsg;

	if (SteamFriends())
	{
		TArray<char> DataArray;
		DataArray.SetNum(1024);

		Result = SteamFriends()->GetFriendMessage(SteamIDFriend, MessageID, DataArray.GetData(), DataArray.Num(), &ChatEntryType);

		if (Result != INDEX_NONE)
		{
			OutText = UTF8_TO_TCHAR(DataArray.GetData());
		}
	}
	OutChatEntryType = static_cast<ESteamChatEntryType>(ChatEntryType);
#endif

	return Result;
}

FString USteamProFriends::GetFriendPersonaName(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaName(SteamIDFriend));
	}
#endif

	return Result;
}

FString USteamProFriends::GetFriendPersonaNameHistory(FSteamID SteamIDFriend, int32 PersonaName)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetFriendPersonaNameHistory(SteamIDFriend, PersonaName));
	}
#endif

	return Result;
}

ESteamPersonaState USteamProFriends::GetFriendPersonaState(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	ESteamPersonaState Result = ESteamPersonaState::Max;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = static_cast<ESteamPersonaState>(SteamFriends()->GetFriendPersonaState(SteamIDFriend));
	}
#endif

	return Result;
}

ESteamFriendRelationship USteamProFriends::GetFriendRelationship(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	ESteamFriendRelationship Result = ESteamFriendRelationship::Max;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = static_cast<ESteamFriendRelationship>(SteamFriends()->GetFriendRelationship(SteamIDFriend));
	}
#endif

	return Result;
}

FString USteamProFriends::GetFriendRichPresence(FSteamID SteamIDFriend, FString Key)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetFriendRichPresence(SteamIDFriend, TCHAR_TO_UTF8(*Key)));
	}
#endif

	return Result;
}

FString USteamProFriends::GetFriendRichPresenceKeyByIndex(FSteamID SteamIDFriend, int32 Key)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetFriendRichPresenceKeyByIndex(SteamIDFriend, Key));
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendRichPresenceKeyCount(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendRichPresenceKeyCount(SteamIDFriend);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendsGroupCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendsGroupCount();
	}
#endif

	return Result;
}

FSteamFriendsGroupID USteamProFriends::GetFriendsGroupIDByIndex(int32 FriendGroup)
{
	LogSteamCoreVeryVerbose("");

	FSteamFriendsGroupID Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendsGroupIDByIndex(FriendGroup);
	}
#endif

	return Result;
}

int32 USteamProFriends::GetFriendsGroupMembersCount(FSteamFriendsGroupID FriendsGroupID)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendsGroupMembersCount(FriendsGroupID);
	}
#endif

	return Result;
}

void USteamProFriends::GetFriendsGroupMembersList(FSteamFriendsGroupID FriendsGroupID, TArray<FSteamID>& OutSteamIdMembers, int32 MembersCount)
{
	LogSteamCoreVeryVerbose("");

	OutSteamIdMembers.Empty();

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		TArray<CSteamID> DataArray;
		DataArray.SetNum(MembersCount);

		SteamFriends()->GetFriendsGroupMembersList(FriendsGroupID, DataArray.GetData(), MembersCount);

		for (int32 i = 0; i < MembersCount; i++)
		{
			OutSteamIdMembers.Add(DataArray[i]);
		}
	}
#endif
}

FString USteamProFriends::GetFriendsGroupName(FSteamFriendsGroupID FriendsGroupID)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetFriendsGroupName(FriendsGroupID));
	}
#endif

	return Result;
}

int USteamProFriends::GetFriendSteamLevel(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	int Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetFriendSteamLevel(SteamIDFriend);
	}
#endif

	return Result;
}

UTexture2D* USteamProFriends::GetLargeFriendAvatar(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	return GetAvatar(2, SteamIDFriend);
}

UTexture2D* USteamProFriends::GetMediumFriendAvatar(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	return GetAvatar(1, SteamIDFriend);
}

FString USteamProFriends::GetPersonaName()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
	}
#endif

	return Result;
}

ESteamPersonaState USteamProFriends::GetPersonaState()
{
	LogSteamCoreVeryVerbose("");

	ESteamPersonaState Result = ESteamPersonaState::Max;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = static_cast<ESteamPersonaState>(SteamFriends()->GetPersonaState());
	}
#endif

	return Result;
}

FString USteamProFriends::GetPlayerNickname(FSteamID SteamIDPlayer)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetPlayerNickname(SteamIDPlayer));
	}
#endif

	return Result;
}

UTexture2D* USteamProFriends::GetSmallFriendAvatar(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	return GetAvatar(0, SteamIDFriend);
}

TArray<ESteamUserRestriction> USteamProFriends::GetUserRestrictions()
{
	LogSteamCoreVeryVerbose("");

	TArray<ESteamUserRestriction> DataArray;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		const uint32 Flags = SteamFriends()->GetUserRestrictions();

		for (int32 i = 0; i <= 31; i++)
		{
			if (Flags & 1 << i)
			{
				DataArray.Add(static_cast<ESteamUserRestriction>(i));
			}
		}
	}
#endif

	return DataArray;
}

bool USteamProFriends::HasFriend(FSteamID SteamIDFriend, TArray<ESteamFriendFlags> Flags)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		int Data = 0;

		for (int i = 0; i < Flags.Num(); i++)
		{
			Data |= *Flags[i];
		}

		bResult = SteamFriends()->HasFriend(SteamIDFriend, Data);
	}
#endif

	return bResult;
}

bool USteamProFriends::InviteUserToGame(FSteamID SteamIDFriend, const FString ConnectString)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->InviteUserToGame(SteamIDFriend, TCHAR_TO_UTF8(*ConnectString));
	}
#endif

	return bResult;
}

bool USteamProFriends::IsClanChatAdmin(FSteamID SteamIDClanChat, FSteamID SteamIDUser)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->IsClanChatAdmin(SteamIDClanChat, SteamIDUser);
	}
#endif

	return bResult;
}

bool USteamProFriends::IsClanPublic(FSteamID SteamIDClan)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->IsClanPublic(SteamIDClan);
	}
#endif

	return bResult;
}

bool USteamProFriends::IsClanOfficialGameGroup(FSteamID SteamIDClan)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->IsClanOfficialGameGroup(SteamIDClan);
	}
#endif

	return bResult;
}

bool USteamProFriends::RegisterProtocolInOverlayBrowser(FString Protocol)
{
	LogSteamCoreVerbose("");
	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->RegisterProtocolInOverlayBrowser(TCHAR_TO_UTF8(*Protocol));
	}
#endif

	return bResult;
}

void USteamProFriends::ActivateGameOverlayInviteDialogConnectString(FString ConnectString)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->ActivateGameOverlayInviteDialogConnectString(TCHAR_TO_UTF8(*ConnectString));
	}
#endif
}

bool USteamProFriends::IsClanChatWindowOpenInSteam(FSteamID SteamIDClanChat)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->IsClanChatWindowOpenInSteam(SteamIDClanChat);
	}
#endif

	return bResult;
}

void USteamProFriends::IsFollowing(const FOnIsFollowing& Callback, FSteamID SteamID)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsIsFollowing* Task = new FOnlineAsyncTaskSteamCoreProFriendsIsFollowing(Callback, SteamID);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProFriends::IsUserInSource(FSteamID SteamIDUser, FSteamID SteamIDSource)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->IsUserInSource(SteamIDUser, SteamIDSource);
	}
#endif

	return bResult;
}

void USteamProFriends::JoinClanChatRoom(const FOnJoinClanChatRoom& Callback, FSteamID SteamIDClan)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom* Task = new FOnlineAsyncTaskSteamCoreProFriendsJoinClanChatRoom(Callback, SteamIDClan);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProFriends::LeaveClanChatRoom(FSteamID SteamIDClan)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	return SteamFriends() ? SteamFriends()->LeaveClanChatRoom(SteamIDClan) : false;
#else
	return false;
#endif
}

bool USteamProFriends::OpenClanChatWindowInSteam(FSteamID SteamIDClanChat)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->OpenClanChatWindowInSteam(SteamIDClanChat);
	}
#endif

	return bResult;
}

bool USteamProFriends::ReplyToFriendMessage(FSteamID SteamIDFriend, FString MsgToSend)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->ReplyToFriendMessage(SteamIDFriend, TCHAR_TO_UTF8(*MsgToSend));
	}
#endif

	return bResult;
}

void USteamProFriends::RequestClanOfficerList(const FOnRequestClanOfficerList& Callback, FSteamID SteamIDClan)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProRequestClanOfficerList* Task = new FOnlineAsyncTaskSteamCoreProRequestClanOfficerList(Callback, SteamIDClan);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProFriends::RequestFriendRichPresence(FSteamID SteamIDFriend)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->RequestFriendRichPresence(SteamIDFriend);
	}
#endif
}

bool USteamProFriends::RequestUserInformation(FSteamID SteamIDUser, bool bRequireNameOnly)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->RequestUserInformation(SteamIDUser, bRequireNameOnly);
	}
#endif

	return bResult;
}

bool USteamProFriends::SendClanChatMessage(FSteamID SteamIDClanChat, FString Text)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->SendClanChatMessage(SteamIDClanChat, TCHAR_TO_UTF8(*Text));
	}
#endif

	return bResult;
}

void USteamProFriends::SetInGameVoiceSpeaking(FSteamID SteamIDUser, bool bSpeaking)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->SetInGameVoiceSpeaking(SteamIDUser, bSpeaking);
	}
#endif
}

bool USteamProFriends::SetListenForFriendsMessages(bool bInterceptEnabled)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->SetListenForFriendsMessages(bInterceptEnabled);
	}
#endif

	return bResult;
}

void USteamProFriends::SetPersonaName(const FOnSetPersonaName& Callback, FString Name)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName* Task = new FOnlineAsyncTaskSteamCoreProFriendsSetPersonaName(Callback, Name);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProFriends::SetPlayedWith(FSteamID SteamIDUserPlayedWith)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		SteamFriends()->SetPlayedWith(SteamIDUserPlayedWith);
	}
#endif
}

bool USteamProFriends::SetRichPresence(FString Key, FString Value)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (Key.Len() == 0 || Value.Len() == 0)
	{
		return bResult;
	}

	if (SteamFriends())
	{
		const FTCHARToUTF8 KeyChar(*Key);
		const FTCHARToUTF8 ValueChar(*Value);

		bResult = SteamFriends()->SetRichPresence(KeyChar.Get(), ValueChar.Get());
	}
#endif

	return bResult;
}

void USteamProFriends::RequestEquippedProfileItems(const FOnRequestEquippedProfileItems& Callback, FSteamID SteamID)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems* Task = new FOnlineAsyncTaskSteamCoreProRequestEquippedProfileItems(Callback, SteamID);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProFriends::BHasEquippedProfileItem(FSteamID SteamId, ESteamCommunityProfileItemType ItemType)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		bResult = SteamFriends()->BHasEquippedProfileItem(SteamId, static_cast<ECommunityProfileItemType>(ItemType));
	}
#endif

	return bResult;
}

FString USteamProFriends::GetProfileItemPropertyString(FSteamID SteamId, ESteamCommunityProfileItemType ItemType, ESteamCommunityProfileItemProperty Prop)
{
	LogSteamCoreVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = UTF8_TO_TCHAR(SteamFriends()->GetProfileItemPropertyString(SteamId, static_cast<ECommunityProfileItemType>(ItemType), static_cast<ECommunityProfileItemProperty>(Prop)));
	}
#endif

	return Result;
}

int32 USteamProFriends::GetProfileItemPropertyUint(FSteamID SteamId, ESteamCommunityProfileItemType ItemType, ESteamCommunityProfileItemProperty Prop)
{
	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamFriends())
	{
		Result = SteamFriends()->GetProfileItemPropertyUint(SteamId, static_cast<ECommunityProfileItemType>(ItemType), static_cast<ECommunityProfileItemProperty>(Prop));
	}
#endif

	return Result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProFriends::OnPersonaStateChange(PersonaStateChange_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		PersonaStateChange.Broadcast(Data);
	});
}

void USteamProFriends::OnSetPersonaNameResponse(SetPersonaNameResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SetPersonaNameResponse.Broadcast(Data);
	});
}

void USteamProFriends::OnJoinClanChatRoomCompletionResult(JoinClanChatRoomCompletionResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		JoinClanChatRoomCompletionResult.Broadcast(Data);
	});
}

void USteamProFriends::OnGameOverlayActivated(GameOverlayActivated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameOverlayActivated.Broadcast(Data);
	});
}

void USteamProFriends::OnGameServerChangeRequested(GameServerChangeRequested_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameServerChangeRequested.Broadcast(Data);
	});
}

void USteamProFriends::OnGameLobbyJoinRequested(GameLobbyJoinRequested_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameLobbyJoinRequested.Broadcast(Data);
	});
}

void USteamProFriends::OnAvatarImageLoaded(AvatarImageLoaded_t* pParam)
{
	LogSteamCoreVerbose("");

	FAvatarImageLoaded Data = *pParam;
	Data.Image = GetSteamTexture(Data.m_iImage);

	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		AvatarImageLoaded.Broadcast(Data);
	});
}

void USteamProFriends::OnFriendRichPresenceUpdate(FriendRichPresenceUpdate_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		FriendRichPresenceUpdate.Broadcast(Data);
	});
}

void USteamProFriends::OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameRichPresenceJoinRequested.Broadcast(Data);
	});
}

void USteamProFriends::OnGameConnectedClanChatMsg(GameConnectedClanChatMsg_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameConnectedClanChatMsg.Broadcast(Data);
	});
}

void USteamProFriends::OnGameConnectedChatJoin(GameConnectedChatJoin_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameConnectedChatJoin.Broadcast(Data);
	});
}

void USteamProFriends::OnGameConnectedChatLeave(GameConnectedChatLeave_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameConnectedChatLeave.Broadcast(Data);
	});
}

void USteamProFriends::OnGameConnectedFriendChatMsg(GameConnectedFriendChatMsg_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameConnectedFriendChatMsg.Broadcast(Data);
	});
}

void USteamProFriends::OnClanOfficerListResponse(ClanOfficerListResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ClanOfficerListResponse.Broadcast(Data);
	});
}

void USteamProFriends::OnDownloadClanActivityCountsResult(DownloadClanActivityCountsResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		DownloadClanActivityCountsResult.Broadcast(Data);
	});
}

void USteamProFriends::OnEquippedProfileItemsChanged(EquippedProfileItemsChanged_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		EquippedProfileItemsChanged.Broadcast(Data);
	});
}
#endif
