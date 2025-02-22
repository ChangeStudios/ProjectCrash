/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "OnlineAsyncTaskManagerAsyncTasksSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 
#include "ExternalUI/OnlineExternalUITypesSteamCore.h"

#if WITH_STEAMCORE
void FOnlineAsyncTaskManagerSteamCore::OnlineTick()
{
	check(m_SteamSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId);

	if (m_SteamSubsystem->IsSteamClientAvailable())
	{
		SteamAPI_RunCallbacks();
	}

	if (m_SteamSubsystem->IsSteamServerAvailable())
	{
		SteamGameServer_RunCallbacks();
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnInviteAccepted(GameRichPresenceJoinRequested_t* pParam)
{
	LogSteamCoreVerbose("");
	
	FString ServerAddress;

	FString CmdLineStr = StringCast<TCHAR>(pParam->m_rgchConnect).Get();
	const TCHAR* CmdLine = *CmdLineStr;

	const FString LobbyConnectCmd = TEXT("+connect_lobby");
	int32 ConnectIdx = CmdLineStr.Find(LobbyConnectCmd, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (ConnectIdx != INDEX_NONE)
	{
		const TCHAR* Str = CmdLine + ConnectIdx + LobbyConnectCmd.Len();
		const FString LobbyIdStr = FParse::Token(Str, 0);
		int64 LobbyId = FCString::Strtoui64(*LobbyIdStr, nullptr, 10);
		if (LobbyId > 0)
		{
			ServerAddress = FUniqueNetIdSteam::Create(LobbyId)->ToString();
		}
	}
	else
	{
		const FString ServerConnectCmd = TEXT("+connect");
		ConnectIdx = CmdLineStr.Find(ServerConnectCmd, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (ConnectIdx != INDEX_NONE)
		{
			const TCHAR* Str = CmdLine + ConnectIdx + ServerConnectCmd.Len();
			const FString ServerIpAddrStr = FParse::Token(Str, 0);
			if (!ServerIpAddrStr.IsEmpty())
			{
				ServerAddress = ServerIpAddrStr;
			}
		}
	}

	ServerAddress = FString::Printf(TEXT("SteamConnectIP=%s"), *ServerAddress);
	
	FOnlineAsyncEventSteamCoreInviteAccepted* NewEvent = new FOnlineAsyncEventSteamCoreInviteAccepted(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_steamIDFriend), ServerAddress);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);

}

void FOnlineAsyncTaskManagerSteamCore::OnLobbyInviteAccepted(GameLobbyJoinRequested_t* pParam)
{
	LogSteamCoreVerbose("");
	if (pParam->m_steamIDLobby.IsLobby())
	{
		const FUniqueNetIdSteamRef LobbyId = FUniqueNetIdSteam::Create(pParam->m_steamIDLobby);

		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(m_SteamSubsystem->GetSessionInterface());
		if (SessionInt.IsValid() && !SessionInt->IsMemberOfLobby(*LobbyId))
		{
			FOnlineAsyncEventSteamCoreLobbyInviteAccepted* NewEvent = new FOnlineAsyncEventSteamCoreLobbyInviteAccepted(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_steamIDFriend), *LobbyId);
			LogSteamCoreVerbose("%s", *NewEvent->ToString());
			AddToOutQueue(NewEvent);
		}
		else
		{
			LogSteamCoreWarn("Attempting to accept invite to lobby user is already in, ignoring.");
		}
	}
	else
	{
		LogSteamCoreWarn("OnLobbyInviteAccepted: Invalid LobbyId received.");
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnLobbyEnter(LobbyEnter_t* pParam)
{
	if (SteamMatchmaking()->GetLobbyOwner(pParam->m_ulSteamIDLobby) != SteamUser()->GetSteamID())
	{
		FOnlineAsyncEventSteamLobbyEnter* NewEvent = new FOnlineAsyncEventSteamLobbyEnter(m_SteamSubsystem, *pParam);
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
	FOnlineAsyncEventSteamLobbyChatUpdate* NewEvent = new FOnlineAsyncEventSteamLobbyChatUpdate(m_SteamSubsystem, *pParam);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
	if (pParam->m_ulSteamIDLobby == pParam->m_ulSteamIDMember)
	{
		if (!pParam->m_bSuccess)
		{
			LogSteamCoreVerbose("Lobby %s is no longer available.", *FUniqueNetIdSteam::ToDebugString(pParam->m_ulSteamIDLobby));
		}

		const ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();
		check(SteamMatchmakingPtr);

		FOnlineAsyncEventSteamLobbyUpdate* NewEvent = new FOnlineAsyncEventSteamLobbyUpdate(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_ulSteamIDLobby));
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnUserStatsReceived(UserStatsReceived_t* pParam)
{
	const CGameID GameID(m_SteamSubsystem->GetSteamAppId());
	if (GameID.ToUint64() == pParam->m_nGameID)
	{
		const FUniqueNetIdSteamRef UserId = FUniqueNetIdSteam::Create(pParam->m_steamIDUser);
		if (pParam->m_eResult != k_EResultOK)
		{
			if (pParam->m_eResult == k_EResultFail)
			{
				LogSteamCoreWarn("Failed to obtain steam user stats, user: %s has no stats entries", *UserId->ToDebugString());
			}
			else
			{
				LogSteamCoreWarn("Failed to obtain steam user stats, user: %s error: %s", *UserId->ToDebugString(), *SteamCore::SteamResultString(pParam->m_eResult));
			}
		}

		FOnlineAsyncEventSteamStatsReceived* NewEvent = new FOnlineAsyncEventSteamStatsReceived(m_SteamSubsystem, *UserId, pParam->m_eResult);
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
	else
	{
		LogSteamCoreWarn("Obtained steam user stats, but for wrong game! Ignoring.");
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnUserStatsStored(UserStatsStored_t* pParam)
{
	const CGameID GameID(m_SteamSubsystem->GetSteamAppId());
	if (GameID.ToUint64() == pParam->m_nGameID)
	{
		const FUniqueNetIdSteamRef UserId = FUniqueNetIdSteam::Create(SteamUser()->GetSteamID());
		if (pParam->m_eResult != k_EResultOK)
		{
			if (pParam->m_eResult == k_EResultInvalidParam)
			{
				LogSteamCoreWarn("Invalid stats data set, stats have been reverted to state prior to last write.");
			}
			else
			{
				LogSteamCoreWarn("Failed to store steam user stats, error: %s", *SteamCore::SteamResultString(pParam->m_eResult));
			}
		}

		FOnlineAsyncEventSteamStatsStored* NewEvent = new FOnlineAsyncEventSteamStatsStored(m_SteamSubsystem, *UserId, pParam->m_eResult);
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
	else
	{
		LogSteamCoreWarn("Stored steam user stats, but for wrong game! Ignoring.");
	}
}

class FOnlineAsyncEventSteamGetTicketForWebApiResponse : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
private:
	GetTicketForWebApiResponse_t CallbackResults;

public:
	FOnlineAsyncEventSteamGetTicketForWebApiResponse(FOnlineSubsystemSteamCore* InSubsystem, const GetTicketForWebApiResponse_t& InResults)
		: FOnlineAsyncEvent(InSubsystem)
		, CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamGetTicketForWebApiResponse()
	{
	}

	/**
	*	Get a human readable description of task
	*/
	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamGetTicketForWebApiResponse Received code %d."), (int32)CallbackResults.m_eResult);
	}

	/**
	* Give the async task a chance to marshal its data back to the game thread
	* Can only be called on the game thread by the async task manager
	*/
	virtual void Finalize() override
	{
		FOnlineAuthSteamCorePtr AuthInt = StaticCastSharedPtr<FOnlineAuthSteamCore>(Subsystem->GetAuthInterface());
		if (AuthInt.IsValid())
		{
			FString ResultToken = BytesToHex(CallbackResults.m_rgubTicket, CallbackResults.m_cubTicket);
			AuthInt->OnGetTicketForWebResponse(CallbackResults.m_hAuthTicket, ResultToken);
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("Auth interface is not valid!"));
		}
	}
};

void FOnlineAsyncTaskManagerSteamCore::OnGetTicketForWebApiResponse(GetTicketForWebApiResponse_t* CallbackData)
{
	FOnlineAsyncEventSteamGetTicketForWebApiResponse* NewEvent = new FOnlineAsyncEventSteamGetTicketForWebApiResponse(m_SteamSubsystem, *CallbackData);
	UE_LOG_ONLINE(Verbose, TEXT("%s"), *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnUserStatsUnloaded(UserStatsUnloaded_t* pParam)
{
	FOnlineAsyncEventSteamStatsUnloaded* NewEvent = new FOnlineAsyncEventSteamStatsUnloaded(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_steamIDUser));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnExternalUITriggered(GameOverlayActivated_t* pParam)
{
	FOnlineAsyncEventSteamCoreExternalUITriggered* NewEvent = new FOnlineAsyncEventSteamCoreExternalUITriggered(m_SteamSubsystem, (pParam->m_bActive != 0) ? true : false);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnected(SteamServersConnected_t* pParam)
{
	FOnlineAsyncEventSteamServerConnectionState* NewEvent = new FOnlineAsyncEventSteamServerConnectionState(m_SteamSubsystem, EOnlineServerConnectionStatus::Connected);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamServersDisconnected(SteamServersDisconnected_t* pParam)
{
	FOnlineAsyncEventSteamServerConnectionState* NewEvent = new FOnlineAsyncEventSteamServerConnectionState(m_SteamSubsystem, SteamCore::SteamConnectionResult(pParam->m_eResult));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnectedGS(SteamServersConnected_t* pParam)
{
	FOnlineAsyncEventSteamServerConnectedGS* NewEvent = new FOnlineAsyncEventSteamServerConnectedGS(m_SteamSubsystem, *FUniqueNetIdSteam::Create(SteamGameServer()->GetSteamID()));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamServersDisconnectedGS(SteamServersDisconnected_t* pParam)
{
	FOnlineAsyncEventSteamServerDisconnectedGS* NewEvent = new FOnlineAsyncEventSteamServerDisconnectedGS(m_SteamSubsystem, *pParam);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamServersConnectFailureGS(SteamServerConnectFailure_t* CallbackData)
{
	if (!CallbackData->m_bStillRetrying)
	{
		FOnlineAsyncEventSteamServerFailedGS* NewEvent = new FOnlineAsyncEventSteamServerFailedGS(m_SteamSubsystem, *CallbackData);
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnPolicyResponseGS(GSPolicyResponse_t* CallbackData)
{
	FOnlineAsyncEventSteamServerPolicyResponseGS* NewEvent = new FOnlineAsyncEventSteamServerPolicyResponseGS(m_SteamSubsystem, *CallbackData);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}


void FOnlineAsyncTaskManagerSteamCore::OnAuthenticationResponseGS(ValidateAuthTicketResponse_t* pParam)
{
	FOnlineAsyncEventSteamAuthenticationResponse* NewEvent = new FOnlineAsyncEventSteamAuthenticationResponse(m_SteamSubsystem, *pParam, true);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnAuthenticationResponse(ValidateAuthTicketResponse_t* pParam)
{
	FOnlineAsyncEventSteamAuthenticationResponse* NewEvent = new FOnlineAsyncEventSteamAuthenticationResponse(m_SteamSubsystem, *pParam, false);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnP2PSessionRequest(P2PSessionRequest_t* pParam)
{
	LogSteamCoreVerbose("Client connection request Id: %s", *FUniqueNetIdSteam::ToDebugString(pParam->m_steamIDRemote));

	IOnlineSessionPtr SessionInt = m_SteamSubsystem->GetSessionInterface();
	if (SessionInt.IsValid() && SessionInt->GetNumSessions() > 0)
	{
		FOnlineAsyncEventSteamConnectionRequest* NewEvent = new FOnlineAsyncEventSteamConnectionRequest(m_SteamSubsystem, SteamNetworking(), *FUniqueNetIdSteam::Create(pParam->m_steamIDRemote));
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
}

void FOnlineAsyncTaskManagerSteamCore::OnP2PSessionConnectFail(P2PSessionConnectFail_t* pParam)
{
	FOnlineAsyncEventSteamConnectionFailed* NewEvent = new FOnlineAsyncEventSteamConnectionFailed(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_steamIDRemote), static_cast<EP2PSessionError>(pParam->m_eP2PSessionError));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnP2PSessionRequestGS(P2PSessionRequest_t* pParam)
{
	FOnlineAsyncEventSteamConnectionRequest* NewEvent = new FOnlineAsyncEventSteamConnectionRequest(m_SteamSubsystem, SteamGameServerNetworking(), *FUniqueNetIdSteam::Create(pParam->m_steamIDRemote));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnP2PSessionConnectFailGS(P2PSessionConnectFail_t* pParam)
{
	FOnlineAsyncEventSteamConnectionFailed* NewEvent = new FOnlineAsyncEventSteamConnectionFailed(m_SteamSubsystem, *FUniqueNetIdSteam::Create(pParam->m_steamIDRemote), static_cast<EP2PSessionError>(pParam->m_eP2PSessionError));
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnSteamShutdown(SteamShutdown_t* pParam)
{
	FOnlineAsyncEventSteamShutdown* NewEvent = new FOnlineAsyncEventSteamShutdown(m_SteamSubsystem);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnRichPresenceUpdate(FriendRichPresenceUpdate_t* pParam)
{
	FOnlineAsyncEventSteamRichPresenceUpdate* NewEvent = new FOnlineAsyncEventSteamRichPresenceUpdate(m_SteamSubsystem, pParam->m_steamIDFriend);
	LogSteamCoreVerbose("%s", *NewEvent->ToString());
	AddToOutQueue(NewEvent);
}

void FOnlineAsyncTaskManagerSteamCore::OnFriendStatusUpdate(PersonaStateChange_t* pParam)
{
	const int ChangedData = pParam->m_nChangeFlags;
	const int RichPresenceWatchedEvents = (k_EPersonaChangeGameServer | k_EPersonaChangeGamePlayed | k_EPersonaChangeStatus | k_EPersonaChangeGoneOffline | k_EPersonaChangeComeOnline);

	if (ChangedData & RichPresenceWatchedEvents)
	{
		FOnlineAsyncEventSteamRichPresenceUpdate* NewEvent = new FOnlineAsyncEventSteamRichPresenceUpdate(m_SteamSubsystem, pParam->m_ulSteamID);
		LogSteamCoreVerbose("%s", *NewEvent->ToString());
		AddToOutQueue(NewEvent);
	}
}
#endif
