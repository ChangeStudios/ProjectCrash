/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCorePrivate.h"
#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemSteamCorePackage.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskSteamCore : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteamCore>
{
PACKAGE_SCOPE:
	FOnlineAsyncTaskSteamCore()
		: FOnlineAsyncTaskBasic(nullptr),
		  m_CallbackHandle(k_uAPICallInvalid)
	{
	}

public:
	FOnlineAsyncTaskSteamCore(class FOnlineSubsystemSteamCore* InSteamSubsystem, SteamAPICall_t InCallbackHandle)
		: FOnlineAsyncTaskBasic(InSteamSubsystem),
		  m_CallbackHandle(InCallbackHandle)
	{
	}

	virtual ~FOnlineAsyncTaskSteamCore() override
	{
	}

PACKAGE_SCOPE:
	SteamAPICall_t m_CallbackHandle;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncTaskManagerSteamCore : public FOnlineAsyncTaskManager
{
public:
	FOnlineAsyncTaskManagerSteamCore(FOnlineSubsystemSteamCore* InOnlineSubsystem)
		: m_SteamSubsystem(InOnlineSubsystem),
		  m_OnP2PSessionRequestCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnP2PSessionRequest),
		  m_OnP2PSessionConnectFailCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnP2PSessionConnectFail),
		  m_OnP2PSessionRequestGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnP2PSessionRequestGS),
		  m_OnP2PSessionConnectFailGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnP2PSessionConnectFailGS),
		  m_OnSteamServersConnectedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnected),
		  m_OnSteamServersDisconnectedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamServersDisconnected),
		  m_OnSteamServersConnectedGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnectedGS),
		  m_OnSteamServersDisconnectedGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamServersDisconnectedGS),
		  m_OnSteamServersConnectFailureGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnectFailureGS),
		  m_OnPolicyResponseGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnPolicyResponseGS),
	      m_OnGetTicketForWebApiResponseCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnGetTicketForWebApiResponse),
		  m_OnAuthenticationResponseGSCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnAuthenticationResponseGS),
		  m_OnAuthenticationResponseCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnAuthenticationResponse),
		  m_OnInviteAcceptedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnInviteAccepted),
		  m_OnLobbyInviteAcceptedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnLobbyInviteAccepted),
		  m_OnLobbyEnterCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnLobbyEnter),
		  m_OnLobbyChatUpdateCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnLobbyChatUpdate),
		  m_OnLobbyDataUpdateCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnLobbyDataUpdate),
		  m_OnUserStatsReceivedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnUserStatsReceived),
		  m_OnUserStatsStoredCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnUserStatsStored),
		  m_OnUserStatsUnloadedCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnUserStatsUnloaded),
		  m_OnExternalUITriggeredCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnExternalUITriggered),
		  m_OnSteamShutdownCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnSteamShutdown),
		  m_OnRichPresenceUpdateCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnRichPresenceUpdate),
		  m_OnFriendStatusUpdateCallback(this, &FOnlineAsyncTaskManagerSteamCore::OnFriendStatusUpdate)
	{
	}

	virtual ~FOnlineAsyncTaskManagerSteamCore() override
	{
	}

	virtual void OnlineTick() override;

protected:
	FOnlineSubsystemSteamCore* m_SteamSubsystem;
	
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnP2PSessionRequest, P2PSessionRequest_t, m_OnP2PSessionRequestCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnP2PSessionConnectFail, P2PSessionConnectFail_t, m_OnP2PSessionConnectFailCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnP2PSessionRequestGS, P2PSessionRequest_t, m_OnP2PSessionRequestGSCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnP2PSessionConnectFailGS, P2PSessionConnectFail_t, m_OnP2PSessionConnectFailGSCallback);

	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamServersConnected, SteamServersConnected_t, m_OnSteamServersConnectedCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamServersDisconnected, SteamServersDisconnected_t, m_OnSteamServersDisconnectedCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamServersConnectedGS, SteamServersConnected_t, m_OnSteamServersConnectedGSCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamServersDisconnectedGS, SteamServersDisconnected_t, m_OnSteamServersDisconnectedGSCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamServersConnectFailureGS, SteamServerConnectFailure_t, m_OnSteamServersConnectFailureGSCallback);
	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnPolicyResponseGS, GSPolicyResponse_t, m_OnPolicyResponseGSCallback);

	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t, m_OnGetTicketForWebApiResponseCallback);

	STEAM_GAMESERVER_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnAuthenticationResponseGS, ValidateAuthTicketResponse_t, m_OnAuthenticationResponseGSCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnAuthenticationResponse, ValidateAuthTicketResponse_t, m_OnAuthenticationResponseCallback);

	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnInviteAccepted, GameRichPresenceJoinRequested_t, m_OnInviteAcceptedCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnLobbyInviteAccepted, GameLobbyJoinRequested_t, m_OnLobbyInviteAcceptedCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnLobbyEnter, LobbyEnter_t, m_OnLobbyEnterCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnLobbyChatUpdate, LobbyChatUpdate_t, m_OnLobbyChatUpdateCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnLobbyDataUpdate, LobbyDataUpdate_t, m_OnLobbyDataUpdateCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnUserStatsReceived, UserStatsReceived_t, m_OnUserStatsReceivedCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnUserStatsStored, UserStatsStored_t, m_OnUserStatsStoredCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnUserStatsUnloaded, UserStatsUnloaded_t, m_OnUserStatsUnloadedCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnExternalUITriggered, GameOverlayActivated_t, m_OnExternalUITriggeredCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnSteamShutdown, SteamShutdown_t, m_OnSteamShutdownCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnRichPresenceUpdate, FriendRichPresenceUpdate_t, m_OnRichPresenceUpdateCallback);
	STEAM_CALLBACK(FOnlineAsyncTaskManagerSteamCore, OnFriendStatusUpdate, PersonaStateChange_t, m_OnFriendStatusUpdateCallback);

};
#endif

