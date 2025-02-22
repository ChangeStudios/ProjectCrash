/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlinePresenceTypesSteamCore.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlinePresenceSteamCore : public IOnlinePresence
{
private:
	FOnlinePresenceSteamCore();

PACKAGE_SCOPE:
	FOnlinePresenceSteamCore(class FOnlineSubsystemSteamCore* InSubsystem);
	void UpdatePresenceForUser(const FUniqueNetId& User);

public:
	//~ Begin IOnlinePresence Interface
	virtual void SetPresence(const FUniqueNetId& User, const FOnlineUserPresenceStatus& Status, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual void QueryPresence(const FUniqueNetId& User, const FOnPresenceTaskCompleteDelegate& Delegate = FOnPresenceTaskCompleteDelegate()) override;
	virtual EOnlineCachedResult::Type GetCachedPresence(const FUniqueNetId& User, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	virtual EOnlineCachedResult::Type GetCachedPresenceForApp(const FUniqueNetId& LocalUserId, const FUniqueNetId& User, const FString& AppId, TSharedPtr<FOnlineUserPresence>& OutPresence) override;
	//~ End IOnlinePresence Interface

	virtual ~FOnlinePresenceSteamCore() override
	{
	}
	
private:
	ISteamFriends* m_SteamFriendsPtr;
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	TUniqueNetIdMap<TSharedRef<FOnlineUserPresenceSteamCore>> m_CachedPresence;
	TUniqueNetIdMap<TSharedRef<const FOnPresenceTaskCompleteDelegate>> m_DelayedPresenceDelegates;
};

typedef TSharedPtr<FOnlinePresenceSteamCore, ESPMode::ThreadSafe> FOnlinePresenceSteamCorePtr;
#endif