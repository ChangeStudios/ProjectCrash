/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Friends/OnlineFriendsInterfaceSteamCore.h"
#include "OnlineFriendsAsyncTasksSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE

FOnlineFriendSteamCore::FOnlineFriendSteamCore(const CSteamID& InUserId)
	: m_UserId(FUniqueNetIdSteam::Create(InUserId))
{
	LogSteamCoreVerbose("");
}

FUniqueNetIdRef FOnlineFriendSteamCore::GetUserId() const
{
	LogSteamCoreVeryVerbose("");
	return m_UserId;
}

FString FOnlineFriendSteamCore::GetRealName() const
{
	LogSteamCoreVeryVerbose("");
	FString Result;
	GetAccountData(TEXT("nickname"), Result);
	return Result;
}

FString FOnlineFriendSteamCore::GetDisplayName(const FString& Platform) const
{
	LogSteamCoreVeryVerbose("");
	FString Result;
	GetAccountData(TEXT("nickname"), Result);
	return Result;
}

bool FOnlineFriendSteamCore::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	LogSteamCoreVeryVerbose("");
	return GetAccountData(AttrName, OutAttrValue);
}

EInviteStatus::Type FOnlineFriendSteamCore::GetInviteStatus() const
{
	LogSteamCoreVeryVerbose("");
	return EInviteStatus::Accepted;
}

const FOnlineUserPresence& FOnlineFriendSteamCore::GetPresence() const
{
	LogSteamCoreVeryVerbose("");
	return m_Presence;
}

FOnlineFriendsSteamCore::FOnlineFriendsSteamCore()
	: m_SteamSubsystem(nullptr), m_SteamUserPtr(nullptr), m_SteamFriendsPtr(nullptr)
{
}

FOnlineFriendsSteamCore::FOnlineFriendsSteamCore(FOnlineSubsystemSteamCore* InSteamSubsystem)
	: m_SteamSubsystem(InSteamSubsystem),
	  m_SteamUserPtr(nullptr),
	  m_SteamFriendsPtr(nullptr)
{
	check(m_SteamSubsystem);
	m_SteamUserPtr = SteamUser();
	m_SteamFriendsPtr = SteamFriends();
}

bool FOnlineFriendsSteamCore::ReadFriendsList(int32 LocalUserNum, const FString& ListName, const FOnReadFriendsListComplete& Delegate)
{
	LogSteamCoreVerbose("");
	FString ErrorStr;
	if (LocalUserNum < MAX_LOCAL_PLAYERS &&
		m_SteamUserPtr != nullptr &&
		m_SteamUserPtr->BLoggedOn() &&
		m_SteamFriendsPtr != nullptr)
	{
		m_SteamSubsystem->QueueAsyncTask(new FOnlineAsyncTaskSteamCoreReadFriendsList(this, LocalUserNum, ListName, Delegate));
	}
	else
	{
		ErrorStr = FString::Printf(TEXT("No valid LocalUserNum=%d"), LocalUserNum);
	}
	if (!ErrorStr.IsEmpty())
	{
		Delegate.ExecuteIfBound(LocalUserNum, false, ListName, ErrorStr);
		return false;
	}
	return true;
}

bool FOnlineFriendsSteamCore::DeleteFriendsList(int32 LocalUserNum, const FString& ListName, const FOnDeleteFriendsListComplete& Delegate /*= FOnDeleteFriendsListComplete()*/)
{
	LogSteamCoreVerbose("");
	Delegate.ExecuteIfBound(LocalUserNum, false, ListName, FString(TEXT("DeleteFriendsList() is not supported")));
	return false;
}

bool FOnlineFriendsSteamCore::SendInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnSendInviteComplete& Delegate /*= FOnSendInviteComplete()*/)
{
	LogSteamCoreVerbose("");
	Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, FString(TEXT("SendInvite() is not supported")));
	return false;
}

bool FOnlineFriendsSteamCore::AcceptInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnAcceptInviteComplete& Delegate /*= FOnAcceptInviteComplete()*/)
{
	LogSteamCoreVerbose("");
	Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, FString(TEXT("AcceptInvite() is not supported")));
	return false;
}

bool FOnlineFriendsSteamCore::RejectInvite(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	LogSteamCoreVerbose("");
	TriggerOnRejectInviteCompleteDelegates(LocalUserNum, false, FriendId, ListName, FString(TEXT("RejectInvite() is not supported")));
	return false;
}

void FOnlineFriendsSteamCore::SetFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FString& Alias, const FOnSetFriendAliasComplete& Delegate /*= FOnSetFriendAliasComplete()*/)
{
	LogSteamCoreVerbose("");
	FUniqueNetIdRef FriendIdRef = FriendId.AsShared();
	m_SteamSubsystem->ExecuteNextTick([LocalUserNum, FriendIdRef, ListName, Delegate]()
	{
		LogSteamCoreWarn("FOnlineFriendsSteamCore::SetFriendAlias is not supported");
		Delegate.ExecuteIfBound(LocalUserNum, *FriendIdRef, ListName, FOnlineError(EOnlineErrorResult::NotImplemented));
	});
}

void FOnlineFriendsSteamCore::DeleteFriendAlias(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnDeleteFriendAliasComplete& Delegate)
{
	LogSteamCoreVerbose("");
	FUniqueNetIdRef FriendIdRef = FriendId.AsShared();
	m_SteamSubsystem->ExecuteNextTick([LocalUserNum, FriendIdRef, ListName, Delegate]()
	{
		LogSteamCoreWarn("FOnlineFriendsSteamCore::DeleteFriendAlias is not supported");
		Delegate.ExecuteIfBound(LocalUserNum, *FriendIdRef, ListName, FOnlineError(EOnlineErrorResult::NotImplemented));
	});
}

bool FOnlineFriendsSteamCore::DeleteFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	LogSteamCoreVerbose("");
	TriggerOnDeleteFriendCompleteDelegates(LocalUserNum, false, FriendId, ListName, FString(TEXT("DeleteFriend() is not supported")));
	return false;
}

bool FOnlineFriendsSteamCore::GetFriendsList(int32 LocalUserNum, const FString& ListName, TArray<TSharedRef<FOnlineFriend>>& OutFriends)
{
	LogSteamCoreVerbose("");
	bool bResult = false;
	if (LocalUserNum < MAX_LOCAL_PLAYERS &&
		m_SteamUserPtr != nullptr &&
		m_SteamUserPtr->BLoggedOn() &&
		m_SteamFriendsPtr != nullptr)
	{
		FSteamFriendsList* FriendsList = m_FriendsLists.Find(LocalUserNum);
		if (FriendsList != nullptr)
		{
			for (int32 FriendIdx = 0; FriendIdx < FriendsList->m_Friends.Num(); FriendIdx++)
			{
				OutFriends.Add(FriendsList->m_Friends[FriendIdx]);
			}
			bResult = true;
		}
	}
	return bResult;
}

TSharedPtr<FOnlineFriend> FOnlineFriendsSteamCore::GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	LogSteamCoreVerbose("");
	TSharedPtr<FOnlineFriend> Result;
	if (LocalUserNum < MAX_LOCAL_PLAYERS &&
		m_SteamUserPtr != nullptr &&
		m_SteamUserPtr->BLoggedOn() &&
		m_SteamFriendsPtr != nullptr)
	{
		FSteamFriendsList* FriendsList = m_FriendsLists.Find(LocalUserNum);
		if (FriendsList != nullptr)
		{
			for (int32 FriendIdx = 0; FriendIdx < FriendsList->m_Friends.Num(); FriendIdx++)
			{
				if (*FriendsList->m_Friends[FriendIdx]->GetUserId() == FriendId)
				{
					Result = FriendsList->m_Friends[FriendIdx];
					break;
				}
			}
		}
	}
	return Result;
}

bool FOnlineFriendsSteamCore::IsFriend(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName)
{
	LogSteamCoreVerbose("");
	bool bIsFriend = false;
	if (LocalUserNum < MAX_LOCAL_PLAYERS &&
		m_SteamUserPtr != nullptr &&
		m_SteamUserPtr->BLoggedOn() &&
		m_SteamFriendsPtr != nullptr)
	{
		const CSteamID SteamPlayerId(*(uint64*)FriendId.GetBytes());
		bIsFriend = m_SteamFriendsPtr->GetFriendRelationship(SteamPlayerId) == k_EFriendRelationshipFriend;
	}
	return bIsFriend;
}

bool FOnlineFriendsSteamCore::QueryRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace)
{
	LogSteamCoreVerbose("");
	LogSteamCoreVerbose("FOnlineFriendsSteamCore::QueryRecentPlayers()");

	TriggerOnQueryRecentPlayersCompleteDelegates(UserId, Namespace, false, TEXT("not implemented"));

	return false;
}

bool FOnlineFriendsSteamCore::GetRecentPlayers(const FUniqueNetId& UserId, const FString& Namespace, TArray<TSharedRef<FOnlineRecentPlayer>>& OutRecentPlayers)
{
	LogSteamCoreVerbose("");
	return false;
}

void FOnlineFriendsSteamCore::DumpRecentPlayers() const
{
}

bool FOnlineFriendsSteamCore::BlockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineFriendsSteamCore::UnblockPlayer(int32 LocalUserNum, const FUniqueNetId& PlayerId)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineFriendsSteamCore::QueryBlockedPlayers(const FUniqueNetId& UserId)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FOnlineFriendsSteamCore::GetBlockedPlayers(const FUniqueNetId& UserId, TArray<TSharedRef<FOnlineBlockedPlayer>>& OutBlockedPlayers)
{
	LogSteamCoreVerbose("");
	return false;
}

void FOnlineFriendsSteamCore::DumpBlockedPlayers() const
{
}

bool FOnlineAsyncTaskSteamCoreReadFriendsList::CanAddUserToList(bool bIsOnline, bool bIsPlayingThisGame, bool bIsPlayingGameInSession) const
{
	LogSteamCoreVerbose("");
	switch (m_FriendsListFilter)
	{
	default:
	case EFriendsLists::Default:
		return true;
	case EFriendsLists::OnlinePlayers:
		return bIsOnline;
	case EFriendsLists::InGamePlayers:
		return bIsOnline && bIsPlayingThisGame;
	case EFriendsLists::InGameAndSessionPlayers:
		return bIsOnline && bIsPlayingThisGame && bIsPlayingGameInSession;
	}
}

#endif