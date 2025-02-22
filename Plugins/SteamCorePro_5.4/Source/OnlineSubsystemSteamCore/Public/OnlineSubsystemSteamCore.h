/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemSteamCorePackage.h"

#ifndef STEAMCORE_SUBSYSTEM
#define STEAMCORE_SUBSYSTEM FName(TEXT("STEAMCORE"))
#endif

#if WITH_STEAMCORE

class FOnlineAchievementsSteamCore;
class FOnlineExternalUISteamCore;
class FOnlineFriendsSteamCore;
class FOnlineIdentitySteamCore;
class FOnlineLeaderboardsSteamCore;
class FOnlineSessionSteamCore;
class FOnlineSharedCloudSteamCore;
class FOnlineUserCloudSteamCore;
class FOnlineVoiceSteamCore;
class FOnlinePresenceSteamCore;
class FOnlineAuthSteamCore;
class FOnlineAuthUtilsSteamCore;
class FOnlinePingInterfaceSteamCore;
class FOnlineEncryptedAppTicketSteamCore;
class FSteamCoreServerInstanceHandler;
class FSteamCoreClientInstanceHandler;
class FOnlineAsyncTaskManagerSteamCore;
struct FSteamUserCloudData;

typedef TSharedPtr<FOnlineSessionSteamCore, ESPMode::ThreadSafe> FOnlineSessionSteamCorePtr;
typedef TSharedPtr<FOnlineIdentitySteamCore, ESPMode::ThreadSafe> FOnlineIdentitySteamCorePtr;
typedef TSharedPtr<FOnlineFriendsSteamCore, ESPMode::ThreadSafe> FOnlineFriendsSteamCorePtr;
typedef TSharedPtr<FOnlineSharedCloudSteamCore, ESPMode::ThreadSafe> FOnlineSharedCloudSteamCorePtr;
typedef TSharedPtr<FOnlineUserCloudSteamCore, ESPMode::ThreadSafe> FOnlineUserCloudSteamCorePtr;
typedef TSharedPtr<FOnlineLeaderboardsSteamCore, ESPMode::ThreadSafe> FOnlineLeaderboardsSteamCorePtr;
typedef TSharedPtr<FOnlineVoiceSteamCore, ESPMode::ThreadSafe> FOnlineVoiceSteamCorePtr;
typedef TSharedPtr<FOnlineExternalUISteamCore, ESPMode::ThreadSafe> FOnlineExternalUISteamCorePtr;
typedef TSharedPtr<FOnlineAchievementsSteamCore, ESPMode::ThreadSafe> FOnlineAchievementsSteamCorePtr;
typedef TSharedPtr<FOnlinePresenceSteamCore, ESPMode::ThreadSafe> FOnlinePresenceSteamCorePtr;
typedef TSharedPtr<FOnlineAuthSteamCore, ESPMode::ThreadSafe> FOnlineAuthSteamCorePtr;
typedef TSharedPtr<FOnlineAuthUtilsSteamCore, ESPMode::ThreadSafe> FOnlineAuthSteamCoreUtilsPtr;
typedef TSharedPtr<FOnlinePingInterfaceSteamCore, ESPMode::ThreadSafe> FOnlinePingSteamCorePtr;
typedef TSharedPtr<FOnlineEncryptedAppTicketSteamCore, ESPMode::ThreadSafe> FOnlineEncryptedAppTicketSteamCorePtr;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSteamServerLoginCompleted, bool /* bWasSuccessful */);
typedef FOnSteamServerLoginCompleted::FDelegate FOnSteamServerLoginCompletedDelegate;

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineSubsystemSteamCore : public FOnlineSubsystemImpl
{
PACKAGE_SCOPE:
	FOnlineSubsystemSteamCore() = delete;

	FOnlineSubsystemSteamCore(FName InInstanceName)
		: FOnlineSubsystemImpl(STEAMCORE_SUBSYSTEM, InInstanceName),
		  m_bSteamworksClientInitialized(false),
		  m_bSteamworksGameServerInitialized(false),
		  m_SteamAppID(0),
		  m_GameServerGamePort(7777),
		  m_GameServerQueryPort(27015),
		  m_SessionInterface(nullptr),
		  m_IdentityInterface(nullptr),
		  m_FriendInterface(nullptr),
		  m_SharedCloudInterface(nullptr),
		  m_UserCloudInterface(nullptr),
		  m_LeaderboardsInterface(nullptr),
		  m_VoiceInterface(nullptr),
		  m_bVoiceInterfaceInitialized(false),
		  m_ExternalUIInterface(nullptr),
		  m_PresenceInterface(nullptr),
		  m_AuthInterface(nullptr),
		  m_AuthInterfaceUtils(nullptr),
		  m_PingInterface(nullptr),
		  m_EncryptedAppTicketInterface(nullptr),
		  m_OnlineAsyncTaskThreadRunnable(nullptr),
		  m_OnlineAsyncTaskThread(nullptr),
		  m_SteamAPIClientHandle(nullptr),
		  m_SteamAPIServerHandle(nullptr)
	{
	}

	bool InitSteamworksClient(bool bRelaunchInSteam, int32 SteamAppId);
	bool InitSteamworksServer();
	void ShutdownSteamworks();
	void QueueAsyncTask(class FOnlineAsyncTask* AsyncTask);
	void QueueAsyncOutgoingItem(class FOnlineAsyncItem* AsyncItem);
	FSteamUserCloudData* GetUserCloudEntry(const FUniqueNetId& UserId);

	bool ClearUserCloudMetadata(const FUniqueNetId& UserId, const FString& Filename);
	void ClearUserCloudFiles();

	FOnlineLeaderboardsSteamCore* GetInternalLeaderboardsInterface();

public:
	virtual ~FOnlineSubsystemSteamCore() override {	}
	virtual FOnlineEncryptedAppTicketSteamCorePtr GetEncryptedAppTicketInterface() const;
	virtual FOnlineAuthSteamCorePtr GetAuthInterface() const;
	virtual FOnlineAuthSteamCoreUtilsPtr GetAuthInterfaceUtils() const;
	virtual FOnlinePingSteamCorePtr GetPingInterface() const;
	virtual void SetPingInterface(FOnlinePingSteamCorePtr InPingInterface);

	// IOnlineSubsystem

	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlinePartyPtr GetPartyInterface() const override;
	virtual IOnlineGroupsPtr GetGroupsInterface() const override;
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;
	virtual IOnlineTimePtr GetTimeInterface() const override;
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override { return nullptr; }
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override { return nullptr; }
	virtual IOnlineEventsPtr GetEventsInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual IOnlineSharingPtr GetSharingInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlineMessagePtr GetMessageInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual IOnlineChatPtr GetChatInterface() const override;
	virtual IOnlineStatsPtr GetStatsInterface() const override;
	virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override;
	virtual IOnlineTournamentPtr GetTournamentInterface() const override;
	virtual bool IsLocalPlayer(const FUniqueNetId& UniqueId) const override;
	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual bool IsEnabled() const override;
	virtual FString GetAppId() const override;
	virtual FText GetOnlineServiceName() const override;

	virtual bool Tick(float DeltaTime) override;

	inline bool IsSteamClientAvailable()
	{
		return m_bSteamworksClientInitialized;
	}

	inline bool IsSteamServerAvailable()
	{
		return m_bSteamworksGameServerInitialized;
	}

	inline uint32 GetSteamAppId() const
	{
		return m_SteamAppID;
	}

	inline int32 GetGameServerGamePort() const
	{
		return m_GameServerGamePort;
	}

	inline int32 GetGameServerQueryPort() const
	{
		return m_GameServerQueryPort;
	}

	inline bool IsUsingSteamNetworking() const
	{
		return false;
	}

	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnSteamServerLoginCompleted, bool);

PACKAGE_SCOPE:
	FCriticalSection m_UserCloudDataLock;
	
protected:
	bool m_bSteamworksClientInitialized;
	bool m_bSteamworksGameServerInitialized;
	uint32 m_SteamAppID;
	int32 m_GameServerGamePort;
	int32 m_GameServerQueryPort;
	TArray<FSteamUserCloudData*> m_UserCloudData;
	FOnlineSessionSteamCorePtr m_SessionInterface;
	FOnlineIdentitySteamCorePtr m_IdentityInterface;
	FOnlineFriendsSteamCorePtr m_FriendInterface;
	FOnlineSharedCloudSteamCorePtr m_SharedCloudInterface;
	FOnlineUserCloudSteamCorePtr m_UserCloudInterface;
	FOnlineLeaderboardsSteamCorePtr m_LeaderboardsInterface;
	mutable IOnlineVoicePtr m_VoiceInterface;
	mutable bool m_bVoiceInterfaceInitialized;
	FOnlineExternalUISteamCorePtr m_ExternalUIInterface;
	FOnlineAchievementsSteamCorePtr m_AchievementsInterface;
	FOnlinePresenceSteamCorePtr m_PresenceInterface;
	FOnlineAuthSteamCorePtr m_AuthInterface;
	FOnlineAuthSteamCoreUtilsPtr m_AuthInterfaceUtils;
	FOnlinePingSteamCorePtr m_PingInterface;
	FOnlineEncryptedAppTicketSteamCorePtr m_EncryptedAppTicketInterface;
	FOnlineAsyncTaskManagerSteamCore* m_OnlineAsyncTaskThreadRunnable;
	FRunnableThread* m_OnlineAsyncTaskThread;
	TSharedPtr<FSteamCoreClientInstanceHandler> m_SteamAPIClientHandle;
	TSharedPtr<FSteamCoreServerInstanceHandler> m_SteamAPIServerHandle;
};

typedef TSharedPtr<FOnlineSubsystemSteamCore, ESPMode::ThreadSafe> FOnlineSubsystemSteamCorePtr;
#endif