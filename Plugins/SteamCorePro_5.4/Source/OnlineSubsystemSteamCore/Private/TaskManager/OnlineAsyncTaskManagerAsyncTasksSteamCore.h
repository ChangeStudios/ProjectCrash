/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineSubsystemSteamCoreUtilities.h"
#include "OnlineSubsystemSteamCore.h"
#include "Sessions/OnlineSessionInterfaceSteamCore.h"
#include "Sessions/OnlineSessionAsyncLobbySteamCore.h"
#include "Leaderboards/OnlineLeaderboardInterfaceSteamCore.h"
#include "Sockets/SocketSubsystemSteamCore.h"
#include "Auth/OnlineAuthInterfaceSteamCore.h"
#include "Presence/OnlinePresenceInterfaceSteamCore.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamLobbyEnter : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamLobbyEnter()
		: FOnlineAsyncEvent(nullptr)
	{
		FMemory::Memzero(m_CallbackResults);
	}

public:
	FOnlineAsyncEventSteamLobbyEnter(FOnlineSubsystemSteamCore* InSubsystem, const LobbyEnter_t& InResults)
		: FOnlineAsyncEvent(InSubsystem),
		  m_CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamLobbyEnter() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamLobbyEnter LobbyId: %s Result: %s"), *FUniqueNetIdSteam::ToDebugString(CSteamID(m_CallbackResults.m_ulSteamIDLobby)), *SteamCore::SteamChatRoomEnterResponseString(static_cast<EChatRoomEnterResponse>(m_CallbackResults.m_EChatRoomEnterResponse)));
	}

	virtual void Finalize() override
	{
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
		if (SessionInt.IsValid())
		{
			const FUniqueNetIdSteamRef LobbyId = FUniqueNetIdSteam::Create(m_CallbackResults.m_ulSteamIDLobby);
			const FNamedOnlineSession* Session = SessionInt->GetNamedSessionFromLobbyId(*LobbyId);
			if (!Session)
			{
				LogSteamCoreWarn("Entered lobby %s, but not found in sessions list", *LobbyId->ToDebugString());
			}
		}
	}
private:
	LobbyEnter_t m_CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamLobbyChatUpdate : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamLobbyChatUpdate()
		: FOnlineAsyncEvent(nullptr)
	{
		FMemory::Memzero(CallbackResults);
	}

public:
	FOnlineAsyncEventSteamLobbyChatUpdate(FOnlineSubsystemSteamCore* InSubsystem, const LobbyChatUpdate_t& InResults)
		: FOnlineAsyncEvent(InSubsystem),
		  CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamLobbyChatUpdate() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamLobbyChatUpdate User: %s Instigator: %s Result: %s"), *FUniqueNetIdSteam::ToDebugString(CSteamID(CallbackResults.m_ulSteamIDUserChanged)), *FUniqueNetIdSteam::ToDebugString(CSteamID(CallbackResults.m_ulSteamIDMakingChange)), *SteamCore::SteamChatMemberStateChangeString(static_cast<EChatMemberStateChange>(CallbackResults.m_rgfChatMemberStateChange)));
	}

	virtual void Finalize() override
	{
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
		if (SessionInt.IsValid())
		{
			const FUniqueNetIdSteamRef LobbyId = FUniqueNetIdSteam::Create(CallbackResults.m_ulSteamIDLobby);
			if (FNamedOnlineSession* Session = SessionInt->GetNamedSessionFromLobbyId(*LobbyId))
			{
				if (!SteamCore::FillMembersFromLobbyData(*LobbyId, *Session))
				{
					LogSteamCoreWarn("Failed to parse session %s member update %s", *Session->SessionName.ToString(), *LobbyId->ToDebugString());
				}
			}
			else
			{
				LogSteamCoreWarn("Received lobby chat update %s, but not found in sessions list", *LobbyId->ToDebugString());
			}
		}
	}
	
private:
	LobbyChatUpdate_t CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamLobbyUpdate : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamLobbyUpdate() = delete;

public:
	FOnlineAsyncEventSteamLobbyUpdate(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InLobbyId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_LobbyId(InLobbyId.AsShared())
	{
	}

	virtual ~FOnlineAsyncEventSteamLobbyUpdate() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamLobbyUpdate LobbyId: %s"), *m_LobbyId->ToDebugString());
	}

	virtual void Finalize() override
	{
		FOnlineAsyncEvent::Finalize();
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
		if (SessionInt.IsValid() && SessionInt->m_CurrentSessionSearch.IsValid() && SessionInt->m_CurrentSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
		{
			SessionInt->m_PendingSearchLobbyIds.AddUnique(m_LobbyId);
		}
		else
		{
			if (FNamedOnlineSession* Session = SessionInt->GetNamedSessionFromLobbyId(*m_LobbyId))
			{
				if (!SteamCore::FillSessionFromLobbyData(Subsystem, *m_LobbyId, *Session) || !SteamCore::FillMembersFromLobbyData(*m_LobbyId, *Session))
				{
					LogSteamCoreWarn("Failed to parse session %s lobby update %s", *Session->SessionName.ToString(), *m_LobbyId->ToDebugString());
				}
			}
			else
			{
				LogSteamCoreWarn("Received lobby update %s, but not found in sessions list", *m_LobbyId->ToDebugString());
			}
		}
	}
private:
	FUniqueNetIdSteamRef m_LobbyId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamStatsReceived : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamStatsReceived() = delete;

public:
	FOnlineAsyncEventSteamStatsReceived(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, const EResult InResult)
		: FOnlineAsyncEvent(InSubsystem),
		  m_UserId(InUserId.AsShared()),
		  m_StatsReceivedResult(InResult)
	{
	}

	virtual ~FOnlineAsyncEventSteamStatsReceived() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamStatsReceived bWasSuccessful: %d User: %s Result: %s"), (m_StatsReceivedResult == k_EResultOK) ? 1 : 0, *m_UserId->ToDebugString(), *SteamCore::SteamResultString(m_StatsReceivedResult));
	}
private:
	const FUniqueNetIdSteamRef m_UserId;
	EResult m_StatsReceivedResult;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamStatsStored : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamStatsStored() = delete;

public:
	FOnlineAsyncEventSteamStatsStored(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId, EResult InResult)
		: FOnlineAsyncEvent(InSubsystem),
		  m_UserId(InUserId.AsShared()),
		  m_StatsStoredResult(InResult)
	{
	}

	virtual ~FOnlineAsyncEventSteamStatsStored() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamStatsStored bWasSuccessful: %d User: %s Result: %s"), (m_StatsStoredResult == k_EResultOK) ? 1 : 0, *m_UserId->ToDebugString(), *SteamCore::SteamResultString(m_StatsStoredResult));
	}

	virtual void Finalize() override
	{
		FOnlineAsyncEvent::Finalize();
		const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(Subsystem->GetLeaderboardsInterface());
		Leaderboards->m_UserStatsStoreStatsFinishedDelegate.ExecuteIfBound(m_StatsStoredResult == k_EResultOK ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed);
	}
private:
	const FUniqueNetIdSteamRef m_UserId;
	EResult m_StatsStoredResult;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamStatsUnloaded : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamStatsUnloaded() = delete;

public:
	FOnlineAsyncEventSteamStatsUnloaded(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InUserId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_UserId(InUserId.AsShared())
	{
	}

	virtual ~FOnlineAsyncEventSteamStatsUnloaded() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamStatsUnloaded UserId: %s"), *m_UserId->ToDebugString());
	}

private:
	FUniqueNetIdSteamRef m_UserId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamServerConnectionState : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamServerConnectionState()
		: m_ConnectionState(EOnlineServerConnectionStatus::NotConnected)
	{
	}

public:
	FOnlineAsyncEventSteamServerConnectionState(FOnlineSubsystemSteamCore* InSubsystem, EOnlineServerConnectionStatus::Type InConnectionState)
		: FOnlineAsyncEvent(InSubsystem),
		  m_ConnectionState(InConnectionState)
	{
	}

	virtual ~FOnlineAsyncEventSteamServerConnectionState() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamServerConnectionState StateChange: %s"), EOnlineServerConnectionStatus::ToString(m_ConnectionState));
	}

	virtual void Finalize() override
	{
		Subsystem->TriggerOnConnectionStatusChangedDelegates(Subsystem->GetSubsystemName().ToString(), EOnlineServerConnectionStatus::Normal, m_ConnectionState);
	}

private:
	const EOnlineServerConnectionStatus::Type m_ConnectionState;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamServerConnectedGS : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamServerConnectedGS() = delete;

public:
	FOnlineAsyncEventSteamServerConnectedGS(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InServerId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_ServerId(InServerId.AsShared())
	{
	}

	virtual ~FOnlineAsyncEventSteamServerConnectedGS() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamServerConnectedGS ServerId: %s"), *m_ServerId->ToDebugString());
	}

	virtual void Finalize() override
	{
		if (Subsystem)
		{
			const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

			if (SessionInt.IsValid())
			{
				SessionInt->m_bSteamworksGameServerConnected = true;
				SessionInt->m_GameServerSteamId = m_ServerId;
				if (Subsystem->IsUsingSteamNetworking())
				{
					if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
					{
						SocketSubsystem->FixupSockets(*SessionInt->m_GameServerSteamId);
					}
				}

				Subsystem->TriggerOnSteamServerLoginCompletedDelegates(true);
			}
		}
	}
private:
	const FUniqueNetIdSteamRef m_ServerId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamServerDisconnectedGS : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamServerDisconnectedGS()
		: m_CallbackResults()
	{
	}

public:
	FOnlineAsyncEventSteamServerDisconnectedGS(FOnlineSubsystemSteamCore* InSubsystem, SteamServersDisconnected_t& InResults)
		: FOnlineAsyncEvent(InSubsystem),
		  m_CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamServerDisconnectedGS() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamServerDisconnectedGS Result: %s"), *SteamCore::SteamResultString(m_CallbackResults.m_eResult));
	}

	virtual void Finalize() override
	{
		bool bTriggerConnectionStatusUpdate = true;
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
		if (SessionInt.IsValid())
		{
			SessionInt->m_bSteamworksGameServerConnected = false;
			SessionInt->m_GameServerSteamId = nullptr;

			const FNamedOnlineSession* Session = SessionInt->GetGameServerSession();
			if (Session && Session->SessionState == EOnlineSessionState::Destroying)
			{
				bTriggerConnectionStatusUpdate = false;
			}
		}

		if (bTriggerConnectionStatusUpdate)
		{
			const EOnlineServerConnectionStatus::Type ConnectionState = SteamCore::SteamConnectionResult(m_CallbackResults.m_eResult);
			Subsystem->TriggerOnConnectionStatusChangedDelegates(Subsystem->GetSubsystemName().ToString(), EOnlineServerConnectionStatus::Normal, ConnectionState);
		}
	}
private:
	SteamServersDisconnected_t m_CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamServerFailedGS : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamServerFailedGS()
		: m_CallbackResults()
	{
	}

public:
	FOnlineAsyncEventSteamServerFailedGS(FOnlineSubsystemSteamCore* InSubsystem, SteamServerConnectFailure_t& InResults)
		: FOnlineAsyncEvent(InSubsystem),
		  m_CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamServerFailedGS() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamServerFailedGS Result: %s"), *SteamCore::SteamResultString(m_CallbackResults.m_eResult));
	}

	virtual void Finalize() override
	{
		if (Subsystem)
		{
			Subsystem->TriggerOnSteamServerLoginCompletedDelegates(false);
		}
	}

private:
	SteamServerConnectFailure_t m_CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamServerPolicyResponseGS : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamServerPolicyResponseGS()
		: m_CallbackResults()
	{
	}

public:
	FOnlineAsyncEventSteamServerPolicyResponseGS(FOnlineSubsystemSteamCore* InSubsystem, GSPolicyResponse_t& InResults)
		: FOnlineAsyncEvent(InSubsystem),
		  m_CallbackResults(InResults)
	{
	}

	virtual ~FOnlineAsyncEventSteamServerPolicyResponseGS() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamServerPolicyResponseGS Secure: %d"), m_CallbackResults.m_bSecure);
	}

	virtual void Finalize() override
	{
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
		
		if (SessionInt && SessionInt.IsValid())
		{
			SessionInt->m_bPolicyResponseReceived = true;
			if (!SessionInt->m_bSteamworksGameServerConnected || !SessionInt->m_GameServerSteamId->IsValid())
			{
				LogSteamCoreWarn("Unexpected GSPolicyResponse callback");
			}
		}
	}
	
private:
	GSPolicyResponse_t m_CallbackResults;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamAuthenticationResponse : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamAuthenticationResponse()
		: m_CallbackResults(), m_bIsServer(false)
	{
	}

public:
	FOnlineAsyncEventSteamAuthenticationResponse(FOnlineSubsystemSteamCore* InSubsystem, const ValidateAuthTicketResponse_t& InResults, const bool bInServerCall)
		: FOnlineAsyncEvent(InSubsystem),
		  m_CallbackResults(InResults),
		  m_bIsServer(bInServerCall)
	{
	}

	virtual ~FOnlineAsyncEventSteamAuthenticationResponse() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamAuthenticationResponse Received code %d. Is server? %d"), static_cast<int32>(m_CallbackResults.m_eAuthSessionResponse), m_bIsServer);
	}

	virtual void Finalize() override
	{
		FOnlineAuthSteamCorePtr AuthInt = StaticCastSharedPtr<FOnlineAuthSteamCore>(Subsystem->GetAuthInterface());
		if (AuthInt.IsValid())
		{
			AuthInt->OnAuthResult(*FUniqueNetIdSteam::Create(m_CallbackResults.m_SteamID), m_CallbackResults.m_eAuthSessionResponse);
		}
		else
		{
			LogSteamCoreWarn("Auth interface is not valid!");
		}
	}

private:
	ValidateAuthTicketResponse_t m_CallbackResults;
	bool m_bIsServer;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamConnectionRequest : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamConnectionRequest() = delete;

public:
	FOnlineAsyncEventSteamConnectionRequest(FOnlineSubsystemSteamCore* InSubsystem, ISteamNetworking* InSteamNetworkingPtr, const FUniqueNetIdSteam& InRemoteId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_SteamNetworkingPtr(InSteamNetworkingPtr),
		  m_RemoteId(InRemoteId.AsShared())
	{
	}

	virtual ~FOnlineAsyncEventSteamConnectionRequest() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamConnectionRequest RemoteId: %s"), *m_RemoteId->ToDebugString());
	}

	virtual void Finalize() override
	{
		if (Subsystem && Subsystem->IsUsingSteamNetworking())
		{
			if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
			{
				if (!SocketSubsystem->AcceptP2PConnection(m_SteamNetworkingPtr, *m_RemoteId))
				{
					LogSteamCoreVerbose("Rejected P2P connection request from %s", *m_RemoteId->ToDebugString());
				}
			}
		}
	}
	
private:
	ISteamNetworking* m_SteamNetworkingPtr;
	FUniqueNetIdSteamRef m_RemoteId;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamConnectionFailed : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamConnectionFailed() = delete;

public:
	FOnlineAsyncEventSteamConnectionFailed(FOnlineSubsystemSteamCore* InSubsystem, const FUniqueNetIdSteam& InRemoteId, const EP2PSessionError InErrorCode)
		: FOnlineAsyncEvent(InSubsystem),
		  m_RemoteId(InRemoteId.AsShared()),
		  m_ErrorCode(InErrorCode)
	{
	}

	virtual ~FOnlineAsyncEventSteamConnectionFailed() override
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamConnectionFailed RemoteId: %s Reason: %s"), *m_RemoteId->ToDebugString(), *SteamCore::SteamP2PConnectError(m_ErrorCode));
	}

	virtual void Finalize() override
	{
		if (Subsystem && Subsystem->IsUsingSteamNetworking())
		{
			if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
			{
				SocketSubsystem->ConnectFailure(*m_RemoteId);
			}
		}
	}

private:
	FUniqueNetIdSteamRef m_RemoteId;
	EP2PSessionError m_ErrorCode;
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamShutdown : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamShutdown()
		: FOnlineAsyncEvent(nullptr)
	{
	}

public:
	FOnlineAsyncEventSteamShutdown(FOnlineSubsystemSteamCore* InSubsystem)
		: FOnlineAsyncEvent(InSubsystem)
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamShutdown shutdown received."));
	}

	virtual void Finalize() override
	{
		FPlatformMisc::RequestExit(false);
	}
};

class ONLINESUBSYSTEMSTEAMCORE_API FOnlineAsyncEventSteamRichPresenceUpdate : public FOnlineAsyncEvent<FOnlineSubsystemSteamCore>
{
	FOnlineAsyncEventSteamRichPresenceUpdate() = delete;
public:
	FOnlineAsyncEventSteamRichPresenceUpdate(FOnlineSubsystemSteamCore* InSubsystem, CSteamID InSteamId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_TargetSteamId(FUniqueNetIdSteam::Create(InSteamId))
	{
	}

	FOnlineAsyncEventSteamRichPresenceUpdate(FOnlineSubsystemSteamCore* InSubsystem, uint64 InSteamId)
		: FOnlineAsyncEvent(InSubsystem),
		  m_TargetSteamId(FUniqueNetIdSteam::Create(InSteamId))
	{
	}

	virtual FString ToString() const override
	{
		return FString::Printf(TEXT("FOnlineAsyncEventSteamRichPresenceUpdate got new information about user %s"), *m_TargetSteamId->ToString());
	}

	virtual void Finalize() override
	{
		const FOnlinePresenceSteamCorePtr PresenceInterface = StaticCastSharedPtr<FOnlinePresenceSteamCore>(Subsystem->GetPresenceInterface());
		if (PresenceInterface && PresenceInterface.IsValid())
		{
			PresenceInterface->UpdatePresenceForUser(*m_TargetSteamId);
		}
	}
private:
	FUniqueNetIdSteamRef m_TargetSteamId;
};
#endif