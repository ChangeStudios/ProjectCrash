/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCore.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineExternalUISteamCore : public IOnlineExternalUI
{
private:
	FOnlineExternalUISteamCore() = delete;

PACKAGE_SCOPE:
	FOnlineExternalUISteamCore(class FOnlineSubsystemSteamCore* InSteamSubsystem)
		: m_bMessageSent(false),
		  m_SteamSubsystem(InSteamSubsystem)
	{
	}

	bool m_bMessageSent;
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	FOnProfileUIClosedDelegate m_ProfileUIClosedDelegate;
	FOnShowWebUrlClosedDelegate m_ShowWebUrlClosedDelegate;
	FOnShowSendMessageUIClosedDelegate m_ShowMessageClosedDelegate;
	FOnShowStoreUIClosedDelegate m_ShowStoreClosedDelegate;

public:
	virtual ~FOnlineExternalUISteamCore() override
	{
	}

	// IOnlineExternalUI
	virtual bool ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate = FOnLoginUIClosedDelegate()) override;
	virtual bool ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate = FOnAccountCreationUIClosedDelegate()) override;
	virtual bool ShowFriendsUI(int32 LocalUserNum) override;
	virtual bool ShowInviteUI(int32 LocalUserNum, FName SessionName = NAME_GameSession) override;
	virtual bool ShowAchievementsUI(int32 LocalUserNum) override;
	virtual bool ShowLeaderboardUI(const FString& LeaderboardName) override;
	virtual bool ShowWebURL(const FString& Url, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate = FOnShowWebUrlClosedDelegate()) override;
	virtual bool CloseWebURL() override;
	virtual bool ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate = FOnProfileUIClosedDelegate()) override;
	virtual bool ShowAccountUpgradeUI(const FUniqueNetId& UniqueId) override;
	virtual bool ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate = FOnShowStoreUIClosedDelegate()) override;
	virtual bool ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate = FOnShowSendMessageUIClosedDelegate()) override;
	virtual bool ShowSendMessageToUserUI(int32 LocalUserNum, const FUniqueNetId& Recipient, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate = FOnShowSendMessageUIClosedDelegate()) override;
};

typedef TSharedPtr<FOnlineExternalUISteamCore, ESPMode::ThreadSafe> FOnlineExternalUISteamCorePtr;
#endif
