/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineAsyncTaskManager.h"
#include "Interfaces/OnlineFriendsInterface.h"

class FOnlineFriendsSteamCore;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCoreReadFriendsList : public FOnlineAsyncTask
{
	bool CanAddUserToList(bool bIsOnline, bool bIsPlayingThisGame, bool bIsPlayingGameInSession) const;
	
public:
	FOnlineAsyncTaskSteamCoreReadFriendsList(FOnlineFriendsSteamCore* InFriendsPtr, int32 InLocalUserNum, const FString& InFriendsListFilter, const FOnReadFriendsListComplete& InDelegate)
		: m_FriendsPtr(InFriendsPtr),
		  m_LocalUserNum(InLocalUserNum),
		  m_Delegate(InDelegate)
	{
		check(m_FriendsPtr);

		if (InFriendsListFilter.Equals(EFriendsLists::ToString(EFriendsLists::OnlinePlayers), ESearchCase::IgnoreCase))
		{
			m_FriendsListFilter = EFriendsLists::OnlinePlayers;
		}
		else if (InFriendsListFilter.Equals(EFriendsLists::ToString(EFriendsLists::InGamePlayers), ESearchCase::IgnoreCase))
		{
			m_FriendsListFilter = EFriendsLists::InGamePlayers;
		}
		else if (InFriendsListFilter.Equals(EFriendsLists::ToString(EFriendsLists::InGameAndSessionPlayers), ESearchCase::IgnoreCase))
		{
			m_FriendsListFilter = EFriendsLists::InGameAndSessionPlayers;
		}
		else
		{
			m_FriendsListFilter = EFriendsLists::Default;
		}
	}

	virtual FString ToString(void) const override
	{
		return TEXT("FOnlineFriendsSteamCore::ReadFriendsList() async task completed successfully");
	}

	virtual bool IsDone(void) const override
	{
		return true;
	}

	virtual bool WasSuccessful(void) const override
	{
		return true;
	}

	virtual void Finalize() override;
	virtual void TriggerDelegates(void) override;

private:
	FOnlineFriendsSteamCore* m_FriendsPtr;
	int32 m_LocalUserNum;
	EFriendsLists::Type m_FriendsListFilter;
	FOnReadFriendsListComplete m_Delegate;
};
