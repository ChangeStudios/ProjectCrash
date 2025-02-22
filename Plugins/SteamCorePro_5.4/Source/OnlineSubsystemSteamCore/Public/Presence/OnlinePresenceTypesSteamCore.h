/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemSteamCoreTypes.h"

const FString DefaultSteamConnectionKey = TEXT("connect");
const FString DefaultSteamPresenceKey = TEXT("status");

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineUserPresenceSteamCore : public FOnlineUserPresence
{
public:
	uint32 m_bIsAFriend:1;

	FOnlineUserPresenceSteamCore()
	{
		Reset();
		m_bIsAFriend = false;
	}

	void Update(const FUniqueNetIdSteam& FriendId);
};
#endif
