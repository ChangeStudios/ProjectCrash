/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlinePresenceInterface.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineFriendSteamCore : public FOnlineFriend
{
public:
	FOnlineFriendSteamCore(const CSteamID& InUserId = CSteamID());

	virtual ~FOnlineFriendSteamCore() override
	{
	};

	virtual FUniqueNetIdRef GetUserId() const override;
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;

	virtual EInviteStatus::Type GetInviteStatus() const override;
	virtual const FOnlineUserPresence& GetPresence() const override;

	inline bool GetAccountData(const FString& Key, FString& OutVal) const
	{
		const FString* FoundVal = m_AccountData.Find(Key);
		if (FoundVal != nullptr)
		{
			OutVal = *FoundVal;
			return true;
		}
		return false;
	}

	FUniqueNetIdRef m_UserId;
	TMap<FString, FString> m_AccountData;
	FOnlineUserPresence m_Presence;
};
#endif
