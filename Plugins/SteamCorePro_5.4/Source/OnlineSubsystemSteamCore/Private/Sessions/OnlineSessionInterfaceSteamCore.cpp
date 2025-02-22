/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Sessions/OnlineSessionInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE
FOnlineSessionInfoSteamCore::FOnlineSessionInfoSteamCore(ESteamSession::Type InSessionType)
	: m_SessionType(InSessionType),
	  m_HostAddr(nullptr),
	  m_SteamP2PAddr(nullptr),
	  m_SessionId(FUniqueNetIdSteam::Create((uint64)0)),
	  m_ConnectionMethod((InSessionType == ESteamSession::LANSession) ? FSteamConnectionMethod::Direct : FSteamConnectionMethod::None)
{
}

FOnlineSessionInfoSteamCore::FOnlineSessionInfoSteamCore(ESteamSession::Type InSessionType, const FUniqueNetIdSteam& InSessionId)
	: m_SessionType(InSessionType),
	  m_HostAddr(nullptr),
	  m_SteamP2PAddr(nullptr),
	  m_SessionId(InSessionId.AsShared()),
	  m_ConnectionMethod(FSteamConnectionMethod::None)
{
}

void FOnlineSessionInfoSteamCore::Init()
{
	LogSteamCoreVerbose("");
}

void FOnlineSessionInfoSteamCore::InitLAN()
{
	LogSteamCoreVerbose("");
	m_SessionType = ESteamSession::LANSession;
	m_ConnectionMethod = FSteamConnectionMethod::Direct;

	uint64 Nonce = 0;
	GenerateNonce(reinterpret_cast<uint8*>(&Nonce), 8);
	m_SessionId = FUniqueNetIdSteam::Create(Nonce);

	bool bCanBindAll;
	m_HostAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	m_HostAddr->SetPort(FURL::UrlConfig.DefaultPort);

	Init();
}

bool FOnlineSessionSteamCore::CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	LogSteamCoreVerbose("");
	
	uint32 Result = ONLINE_FAIL;

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		Session = AddNamedSession(SessionName, NewSessionSettings);
		check(Session);
		Session->SessionState = EOnlineSessionState::Creating;
		Session->NumOpenPrivateConnections = NewSessionSettings.NumPrivateConnections;
		Session->NumOpenPublicConnections = NewSessionSettings.bIsDedicated ? NewSessionSettings.NumPublicConnections : NewSessionSettings.NumPublicConnections - 1;

		Session->HostingPlayerNum = HostingPlayerNum;
		Session->OwningUserId = SteamUser() ? FUniqueNetIdSteamPtr(FUniqueNetIdSteam::Create(SteamUser()->GetSteamID())) : nullptr;
		Session->OwningUserName = SteamFriends() ? SteamFriends()->GetPersonaName() : GetCustomDedicatedServerName();

		Session->SessionSettings.BuildUniqueId = GetBuildUniqueId();

		if (!NewSessionSettings.bIsLANMatch)
		{
			if (Session->SessionSettings.bUseLobbiesIfAvailable)
			{
				Result = CreateLobbySession(HostingPlayerNum, Session);
			}
			else
			{
				Result = CreateInternetSession(HostingPlayerNum, Session);
			}
		}
		else
		{
			Result = CreateLANSession(HostingPlayerNum, Session);
		}

		if (Result != ONLINE_IO_PENDING)
		{
			Session->SessionState = EOnlineSessionState::Pending;

			if (Result != ONLINE_SUCCESS)
			{
				RemoveNamedSession(SessionName);
			}
			else
			{
				RegisterLocalPlayers(Session);
			}
		}
	}
	else
	{
		LogSteamCoreWarn("Cannot create session '%s': session already exists.", *SessionName.ToString());
	}

	if (Result != ONLINE_IO_PENDING)
	{
		TriggerOnCreateSessionCompleteDelegates(SessionName, (Result == ONLINE_SUCCESS) ? true : false);
	}

	return Result == ONLINE_IO_PENDING || Result == ONLINE_SUCCESS;
}

bool FOnlineSessionSteamCore::CreateSession(const FUniqueNetId& HostingPlayerId, FName SessionName, const FOnlineSessionSettings& NewSessionSettings)
{
	LogSteamCoreVerbose("");
	
	return CreateSession(0, SessionName, NewSessionSettings);
}

uint32 FOnlineSessionSteamCore::CreateLobbySession(int32 HostingPlayerNum, FNamedOnlineSession* Session) const
{
	LogSteamCoreVerbose("");
	
	uint32 Result = ONLINE_FAIL;

	if (Session)
	{
		const int32 MaxLobbySize = Session->SessionSettings.NumPrivateConnections + Session->SessionSettings.NumPublicConnections;
		const ELobbyType SteamLobbyType = BuildLobbyTypeSteamCore(&Session->SessionSettings);

		FOnlineAsyncTaskSteamCoreCreateLobby* NewTask = new FOnlineAsyncTaskSteamCoreCreateLobby(m_SteamSubsystem, Session->SessionName, SteamLobbyType, MaxLobbySize);
		m_SteamSubsystem->QueueAsyncTask(NewTask);

		Result = ONLINE_IO_PENDING;
	}
	else
	{
		LogSteamCoreWarn("CreateLobbySession: NULL Session");
	}

	return Result;
}

uint32 FOnlineSessionSteamCore::CreateInternetSession(int32 HostingPlayerNum, FNamedOnlineSession* Session)
{
	LogSteamCoreVerbose("");
	
	uint32 Result = ONLINE_FAIL;

	FNamedOnlineSession* MasterSession = GetGameServerSession();
	if (MasterSession == nullptr)
	{
		if (m_SteamSubsystem->IsSteamServerAvailable())
		{
			m_bPolicyResponseReceived = false;

			FOnlineAsyncTaskSteamCoreCreateServer* NewTask = new FOnlineAsyncTaskSteamCoreCreateServer(m_SteamSubsystem, Session->SessionName);
			m_SteamSubsystem->QueueAsyncTask(NewTask);
			Result = ONLINE_IO_PENDING;
		}
		else
		{
			LogSteamCoreWarn("Failed to initialize game server with Steam!");
		}
	}
	else
	{
		LogSteamCoreVerbose("Advertised session %s already exists, unable to create another.", *Session->SessionName.ToString());
	}

	return Result;
}


uint32 FOnlineSessionSteamCore::CreateLANSession(int32 HostingPlayerNum, FNamedOnlineSession* Session)
{
	LogSteamCoreVerbose("");
	check(Session);
	uint32 Result = ONLINE_SUCCESS;

	FOnlineSessionInfoSteamCore* NewSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::LANSession);
	NewSessionInfo->InitLAN();

	if (!Session->OwningUserId.IsValid())
	{
		Session->OwningUserId = FUniqueNetIdSteam::Create(k_steamIDLanModeGS);
	}
	Session->SessionInfo = MakeShareable(NewSessionInfo);

	if (Session->SessionSettings.bShouldAdvertise)
	{
		if (!m_LANSession)
		{
			m_LANSession = new FLANSession();
		}

		FOnValidQueryPacketDelegate QueryPacketDelegate = FOnValidQueryPacketDelegate::CreateRaw(this, &FOnlineSessionSteamCore::OnValidQueryPacketReceived);
		if (!m_LANSession->Host(QueryPacketDelegate))
		{
			Result = ONLINE_FAIL;
		}
	}

	return Result;
}

bool FOnlineSessionSteamCore::StartSession(FName SessionName)
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;
	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (Session->SessionState == EOnlineSessionState::Pending ||
			Session->SessionState == EOnlineSessionState::Ended)
		{
			if (!Session->SessionSettings.bIsLANMatch)
			{
				Result = ONLINE_SUCCESS;
				Session->SessionState = EOnlineSessionState::InProgress;

				if (SteamFriends() != nullptr)
				{
					for (int32 PlayerIdx = 0; PlayerIdx < Session->RegisteredPlayers.Num(); PlayerIdx++)
					{
						FUniqueNetIdSteam& Player = (FUniqueNetIdSteam&)Session->RegisteredPlayers[PlayerIdx].Get();
						SteamFriends()->SetPlayedWith(Player);
					}
				}
			}
			else
			{
				if (!Session->SessionSettings.bAllowJoinInProgress)
				{
					m_LANSession->StopLANSession();
				}
				Result = ONLINE_SUCCESS;
				Session->SessionState = EOnlineSessionState::InProgress;
			}
		}
		else
		{
			LogSteamCoreWarn("Can't start an online session (%s) in state %s", *SessionName.ToString(), EOnlineSessionState::ToString(Session->SessionState));
		}
	}
	else
	{
		LogSteamCoreWarn("Can't start an online game for session (%s) that hasn't been created", *SessionName.ToString());
	}

	if (Result != ONLINE_IO_PENDING)
	{
		TriggerOnStartSessionCompleteDelegates(SessionName, (Result == ONLINE_SUCCESS) ? true : false);
	}

	return Result == ONLINE_SUCCESS || Result == ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::UpdateSession(FName SessionName, FOnlineSessionSettings& UpdatedSessionSettings, bool bShouldRefreshOnlineData)
{
	LogSteamCoreVerbose("");
	bool bWasSuccessful = true;

	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (!Session->SessionSettings.bIsLANMatch)
		{
			if (const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get()))
			{
				if (SessionInfo->m_SessionType == ESteamSession::LobbySession && SessionInfo->m_SessionId->IsValid())
				{
					FOnlineAsyncTaskSteamCoreUpdateLobby* NewTask = new FOnlineAsyncTaskSteamCoreUpdateLobby(m_SteamSubsystem, SessionName, bShouldRefreshOnlineData, UpdatedSessionSettings);
					m_SteamSubsystem->QueueAsyncTask(NewTask);
				}
				else if (SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionHost)
				{
					FOnlineAsyncTaskSteamCoreUpdateServer* NewTask = new FOnlineAsyncTaskSteamCoreUpdateServer(m_SteamSubsystem, SessionName, bShouldRefreshOnlineData, UpdatedSessionSettings);
					m_SteamSubsystem->QueueAsyncTask(NewTask);
				}
			}
			else
			{
				bWasSuccessful = false;
			}
		}
		else
		{
			Session->SessionSettings = UpdatedSessionSettings;
			TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);
		}
	}

	return bWasSuccessful;
}

bool FOnlineSessionSteamCore::EndSession(FName SessionName)
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;

	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		if (Session->SessionState == EOnlineSessionState::InProgress)
		{
			if (!Session->SessionSettings.bIsLANMatch)
			{
				Result = EndInternetSession(Session);
			}
			else
			{
				if (Session->SessionSettings.bShouldAdvertise &&
					m_LANSession->LanBeacon == nullptr &&
					m_SteamSubsystem->IsServer())
				{
					Result = CreateLANSession(Session->HostingPlayerNum, Session);
				}
				else
				{
					Result = ONLINE_SUCCESS;
				}
			}
		}
		else
		{
			LogSteamCoreWarn("Can't end session (%s) in state %s", *SessionName.ToString(), EOnlineSessionState::ToString(Session->SessionState));
		}
	}
	else
	{
		LogSteamCoreWarn("Can't end an online game for session (%s) that hasn't been created", *SessionName.ToString());
	}

	if (Result != ONLINE_IO_PENDING)
	{
		if (Session)
		{
			Session->SessionState = EOnlineSessionState::Ended;
		}

		TriggerOnEndSessionCompleteDelegates(SessionName, (Result == ONLINE_SUCCESS) ? true : false);
	}

	return Result == ONLINE_SUCCESS || Result == ONLINE_IO_PENDING;
}

uint32 FOnlineSessionSteamCore::EndInternetSession(FNamedOnlineSession* Session) const
{
	LogSteamCoreVerbose("");
	check(Session && Session->SessionState == EOnlineSessionState::InProgress);

	const FOnlineLeaderboardsSteamCorePtr Leaderboards = StaticCastSharedPtr<FOnlineLeaderboardsSteamCore>(m_SteamSubsystem->GetLeaderboardsInterface());
	if (Leaderboards.IsValid())
	{
		Leaderboards->FlushLeaderboards(Session->SessionName);
	}

	Session->SessionState = EOnlineSessionState::Ending;

	FOnlineAsyncTaskSteamCoreEndSession* NewTask = new FOnlineAsyncTaskSteamCoreEndSession(m_SteamSubsystem, Session->SessionName);
	m_SteamSubsystem->QueueAsyncTask(NewTask);

	return ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::DestroySession(FName SessionName, const FOnDestroySessionCompleteDelegate& CompletionDelegate)
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;
	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (Session->SessionState != EOnlineSessionState::Destroying)
		{
			if (!Session->SessionSettings.bIsLANMatch)
			{
				if (Session->SessionState == EOnlineSessionState::InProgress)
				{
					EndInternetSession(Session);
				}

				if (Session->SessionSettings.bUsesPresence)
				{
					Result = DestroyLobbySession(Session, CompletionDelegate);
				}
				else
				{
					Result = DestroyInternetSession(Session, CompletionDelegate);
				}
			}
			else
			{
				if (m_LANSession)
				{
					m_LANSession->StopLANSession();
					delete m_LANSession;
					m_LANSession = nullptr;
				}

				Result = ONLINE_SUCCESS;
			}

			if (Result != ONLINE_IO_PENDING)
			{
				RemoveNamedSession(Session->SessionName);
				CompletionDelegate.ExecuteIfBound(SessionName, (Result == ONLINE_SUCCESS) ? true : false);
				TriggerOnDestroySessionCompleteDelegates(SessionName, (Result == ONLINE_SUCCESS) ? true : false);
			}
		}
		else
		{
			LogSteamCoreWarn("Already in process of destroying session (%s)", *SessionName.ToString());
		}
	}
	else
	{
		LogSteamCoreWarn("Can't destroy a null online session (%s)", *SessionName.ToString());
		CompletionDelegate.ExecuteIfBound(SessionName, false);
		TriggerOnDestroySessionCompleteDelegates(SessionName, false);
	}

	return Result == ONLINE_SUCCESS || Result == ONLINE_IO_PENDING;
}

uint32 FOnlineSessionSteamCore::DestroyLobbySession(FNamedOnlineSession* Session, const FOnDestroySessionCompleteDelegate& CompletionDelegate) const
{
	LogSteamCoreVerbose("");
	Session->SessionState = EOnlineSessionState::Destroying;

	if (Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		check(SessionInfo->m_SessionType == ESteamSession::LobbySession);

		FOnlineAsyncTaskSteamCoreLeaveLobby* NewTask = new FOnlineAsyncTaskSteamCoreLeaveLobby(m_SteamSubsystem, Session->SessionName, *SessionInfo->m_SessionId);
		m_SteamSubsystem->QueueAsyncTask(NewTask);
	}

	FOnlineAsyncTaskSteamCoreDestroySession* NewTask = new FOnlineAsyncTaskSteamCoreDestroySession(m_SteamSubsystem, Session->SessionName, CompletionDelegate);
	m_SteamSubsystem->QueueAsyncTask(NewTask);

	return ONLINE_IO_PENDING;
}

uint32 FOnlineSessionSteamCore::DestroyInternetSession(FNamedOnlineSession* Session, const FOnDestroySessionCompleteDelegate& CompletionDelegate) const
{
	LogSteamCoreVerbose("");
	Session->SessionState = EOnlineSessionState::Destroying;

	if (Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		check(SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionHost || SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionClient);
	}

	const FOnlineAuthSteamCorePtr SteamAuth = m_SteamSubsystem->GetAuthInterface();
	if (SteamUser() != nullptr && SteamUser()->BLoggedOn() && (!SteamAuth.IsValid() || !SteamAuth->IsSessionAuthEnabled()))
	{
		LogSteamCoreWarn("AUTH: DestroyInternetSession is calling the depricated AdvertiseGame call");
		SteamUser()->AdvertiseGame(k_steamIDNil, 0, 0);
	}

	if (m_bSteamworksGameServerConnected && m_GameServerSteamId->IsValid())
	{
		FOnlineAsyncTaskSteamCoreLogoffServer* LogoffTask = new FOnlineAsyncTaskSteamCoreLogoffServer(m_SteamSubsystem, Session->SessionName);
		m_SteamSubsystem->QueueAsyncTask(LogoffTask);
	}

	FOnlineAsyncTaskSteamCoreDestroySession* DestroyTask = new FOnlineAsyncTaskSteamCoreDestroySession(m_SteamSubsystem, Session->SessionName, CompletionDelegate);
	m_SteamSubsystem->QueueAsyncTask(DestroyTask);

	return ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::IsPlayerInSession(FName SessionName, const FUniqueNetId& UniqueId)
{
	return IsPlayerInSessionImpl(this, SessionName, UniqueId);
}

bool FOnlineSessionSteamCore::StartMatchmaking(const TArray<FUniqueNetIdRef>& LocalPlayers, FName SessionName, const FOnlineSessionSettings& NewSessionSettings, TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	LogSteamCoreWarn("StartMatchmaking is not supported on this platform. Use FindSessions or FindSessionById.");
	TriggerOnMatchmakingCompleteDelegates(SessionName, false);
	return false;
}

bool FOnlineSessionSteamCore::CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName)
{
	LogSteamCoreWarn("CancelMatchmaking is not supported on this platform. Use CancelFindSessions.");
	TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
	return false;
}

bool FOnlineSessionSteamCore::CancelMatchmaking(const FUniqueNetId& SearchingPlayerId, FName SessionName)
{
	LogSteamCoreWarn("CancelMatchmaking is not supported on this platform. Use CancelFindSessions.");
	TriggerOnCancelMatchmakingCompleteDelegates(SessionName, false);
	return false;
}

bool FOnlineSessionSteamCore::FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	LogSteamCoreVerbose("");
	uint32 Return = ONLINE_FAIL;

	if (m_SteamSubsystem->IsDedicated())
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::Failed;
		TriggerOnFindSessionsCompleteDelegates(false);
		return false;
	}

	if (!m_CurrentSessionSearch.IsValid() && SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		SearchSettings->SearchResults.Empty();
		m_CurrentSessionSearch = SearchSettings;
		if (SearchSettings->bIsLanQuery == false)
		{
			Return = FindInternetSession(SearchSettings);
		}
		else
		{
			Return = FindLANSession(SearchSettings);
		}

		if (Return == ONLINE_IO_PENDING)
		{
			SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
		}
	}
	else
	{
		LogSteamCoreWarn("Ignoring game search request while one is pending");
		Return = ONLINE_IO_PENDING;
	}

	return Return == ONLINE_SUCCESS || Return == ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::FindSessions(const FUniqueNetId& SearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	return FindSessions(0, SearchSettings);
}

bool FOnlineSessionSteamCore::FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId, const FOnSingleSessionResultCompleteDelegate& CompletionDelegates)
{
	const FOnlineSessionSearchResult EmptyResult;
	CompletionDelegates.ExecuteIfBound(0, false, EmptyResult);
	return true;
}

uint32 FOnlineSessionSteamCore::FindInternetSession(const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	LogSteamCoreVerbose("");
	bool PresenceSearch = false;
	if (SearchSettings->QuerySettings.Get(SEARCH_PRESENCE, PresenceSearch) && PresenceSearch)
	{
		FOnlineAsyncTaskSteamCoreFindLobbies* NewTask = new FOnlineAsyncTaskSteamCoreFindLobbies(m_SteamSubsystem, SearchSettings);
		m_SteamSubsystem->QueueAsyncTask(NewTask);
	}
	else
	{
		FOnlineAsyncTaskSteamCoreFindServers* NewTask = new FOnlineAsyncTaskSteamCoreFindServers(m_SteamSubsystem, SearchSettings, OnFindSessionsCompleteDelegates);
		m_SteamSubsystem->QueueAsyncTask(NewTask);
	}

	return ONLINE_IO_PENDING;
}

uint32 FOnlineSessionSteamCore::FindLANSession(const TSharedRef<FOnlineSessionSearch>& SearchSettings)
{
	LogSteamCoreVerbose("");
	uint32 Return = ONLINE_IO_PENDING;

	bool PresenceSearch = false;
	if (SearchSettings->QuerySettings.Get(SEARCH_PRESENCE, PresenceSearch) && PresenceSearch)
	{
		if (!m_LANSession)
		{
			m_LANSession = new FLANSession();
		}

		GenerateNonce(reinterpret_cast<uint8*>(&m_LANSession->LanNonce), 8);

		FOnValidResponsePacketDelegate ResponseDelegate = FOnValidResponsePacketDelegate::CreateRaw(this, &FOnlineSessionSteamCore::OnValidResponsePacketReceived);
		FOnSearchingTimeoutDelegate TimeoutDelegate = FOnSearchingTimeoutDelegate::CreateRaw(this, &FOnlineSessionSteamCore::OnLANSearchTimeout);

		FNboSerializeToBufferSteamCore Packet(LAN_BEACON_MAX_PACKET_SIZE);
		m_LANSession->CreateClientQueryPacket(Packet, m_LANSession->LanNonce);
		if (Packet.HasOverflow() || m_LANSession->Search(Packet, ResponseDelegate, TimeoutDelegate) == false)
		{
			Return = ONLINE_FAIL;
			delete m_LANSession;
			m_LANSession = nullptr;

			m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Failed;

			TriggerOnFindSessionsCompleteDelegates(false);
		}
	}
	else
	{
		FOnlineAsyncTaskSteamCoreFindServers* NewTask = new FOnlineAsyncTaskSteamCoreFindServers(m_SteamSubsystem, SearchSettings, OnFindSessionsCompleteDelegates);
		m_SteamSubsystem->QueueAsyncTask(NewTask);
	}

	return Return;
}

bool FOnlineSessionSteamCore::CancelFindSessions()
{
	LogSteamCoreVerbose("");
	uint32 Return = ONLINE_FAIL;
	if (m_CurrentSessionSearch.IsValid() && m_CurrentSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		if (m_CurrentSessionSearch->bIsLanQuery)
		{
			Return = ONLINE_SUCCESS;
			m_LANSession->StopLANSession();
			m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Failed;
		}
		else
		{
			Return = ONLINE_SUCCESS;
			m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Failed;
			m_CurrentSessionSearch = nullptr;
		}
	}
	else
	{
		LogSteamCoreWarn("Can't cancel a search that isn't in progress");
	}

	if (Return != ONLINE_IO_PENDING)
	{
		TriggerOnCancelFindSessionsCompleteDelegates(true);
	}

	return Return == ONLINE_SUCCESS || Return == ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::JoinSession(int32 PlayerNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	LogSteamCoreVerbose("");
	uint32 Return = ONLINE_FAIL;
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		Session = AddNamedSession(SessionName, DesiredSession.Session);
		Session->HostingPlayerNum = PlayerNum;

		if (!Session->SessionSettings.bIsLANMatch)
		{
			if (DesiredSession.Session.SessionInfo.IsValid())
			{
				const FOnlineSessionInfoSteamCore* SearchSessionInfo = static_cast<const FOnlineSessionInfoSteamCore*>(DesiredSession.Session.SessionInfo.Get());

				if (DesiredSession.Session.SessionSettings.bUsesPresence)
				{
					FOnlineSessionInfoSteamCore* NewSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::LobbySession, *SearchSessionInfo->m_SessionId);
					Session->SessionInfo = MakeShareable(NewSessionInfo);

					Return = JoinLobbySession(PlayerNum, Session, &DesiredSession.Session);
				}
				else
				{
					FOnlineSessionInfoSteamCore* NewSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::AdvertisedSessionClient, *SearchSessionInfo->m_SessionId);
					Session->SessionInfo = MakeShareable(NewSessionInfo);

					Return = JoinInternetSession(PlayerNum, Session, &DesiredSession.Session);
				}
			}
			else
			{
				LogSteamCoreWarn("Invalid session info on search result", *SessionName.ToString());
			}
		}
		else
		{
			FOnlineSessionInfoSteamCore* NewSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::LANSession);
			Session->SessionInfo = MakeShareable(NewSessionInfo);

			Return = JoinLANSession(PlayerNum, Session, &DesiredSession.Session);
		}

		if (Return != ONLINE_IO_PENDING)
		{
			if (Return != ONLINE_SUCCESS)
			{
				RemoveNamedSession(SessionName);
			}
			else
			{
				RegisterLocalPlayers(Session);
			}
		}
	}
	else
	{
		LogSteamCoreWarn("Session (%s) already exists, can't join twice", *SessionName.ToString());
	}

	if (Return != ONLINE_IO_PENDING)
	{
		TriggerOnJoinSessionCompleteDelegates(SessionName, Return == ONLINE_SUCCESS ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	}

	return Return == ONLINE_SUCCESS || Return == ONLINE_IO_PENDING;
}

bool FOnlineSessionSteamCore::JoinSession(const FUniqueNetId& PlayerId, FName SessionName, const FOnlineSessionSearchResult& DesiredSession)
{
	return JoinSession(0, SessionName, DesiredSession);
}

uint32 FOnlineSessionSteamCore::JoinLobbySession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession) const
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;
	if (Session->SessionInfo.IsValid())
	{
		FOnlineSessionInfoSteamCore* SteamSessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		if (SteamSessionInfo->m_SessionType == ESteamSession::LobbySession && SteamSessionInfo->m_SessionId->IsValid())
		{
			const FOnlineSessionInfoSteamCore* SearchSessionInfo = static_cast<const FOnlineSessionInfoSteamCore*>(SearchSession->SessionInfo.Get());
			SteamSessionInfo->m_HostAddr = SearchSessionInfo->m_HostAddr;
			SteamSessionInfo->m_SteamP2PAddr = SearchSessionInfo->m_SteamP2PAddr;
			SteamSessionInfo->m_ConnectionMethod = SearchSessionInfo->m_ConnectionMethod;

			FOnlineAsyncTaskSteamCoreJoinLobby* NewTask = new FOnlineAsyncTaskSteamCoreJoinLobby(m_SteamSubsystem, Session->SessionName, *SteamSessionInfo->m_SessionId);
			m_SteamSubsystem->QueueAsyncTask(NewTask);
			Result = ONLINE_IO_PENDING;
		}
	}

	return Result;
}

uint32 FOnlineSessionSteamCore::JoinInternetSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;
	Session->SessionState = EOnlineSessionState::Pending;

	if (Session->SessionInfo.IsValid())
	{
		FOnlineSessionInfoSteamCore* SteamSessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		if (SteamSessionInfo->m_SessionType == ESteamSession::AdvertisedSessionClient && SteamSessionInfo->m_SessionId->IsValid())
		{
			const FOnlineSessionInfoSteamCore* SearchSessionInfo = static_cast<const FOnlineSessionInfoSteamCore*>(SearchSession->SessionInfo.Get());
			SteamSessionInfo->m_HostAddr = SearchSessionInfo->m_HostAddr;
			SteamSessionInfo->m_SteamP2PAddr = SearchSessionInfo->m_SteamP2PAddr;
			SteamSessionInfo->m_ConnectionMethod = SearchSessionInfo->m_ConnectionMethod;

			if (SearchSession->SessionSettings.bAllowJoinViaPresence)
			{
				const FString ConnectionString = GetSteamConnectionString(Session->SessionName);

				if (!SteamFriends()->SetRichPresence("connect", TCHAR_TO_UTF8(*ConnectionString)))
				{
					LogSteamCoreVerbose("Failed to set rich presence for session %s", *Session->SessionName.ToString());
				}

				bool bShouldUseFallback = true;
				const FOnlineAuthSteamCorePtr SteamAuth = m_SteamSubsystem->GetAuthInterface();
				if (SteamAuth.IsValid() && SteamAuth->IsSessionAuthEnabled())
				{
					bShouldUseFallback = false;
				}

				if (SteamUser() != nullptr && SteamUser()->BLoggedOn() && bShouldUseFallback)
				{
					uint32 IpAddr;
					uint32 Port = SteamSessionInfo->m_HostAddr->GetPort();
					SteamSessionInfo->m_HostAddr->GetIp(IpAddr);
					SteamUser()->AdvertiseGame(k_steamIDNonSteamGS, IpAddr, Port);
					LogSteamCoreWarn("AUTH: JoinInternetSession is calling the depricated AdvertiseGame call");
				}
			}
			Result = ONLINE_SUCCESS;
		}
	}

	return Result;
}

uint32 FOnlineSessionSteamCore::JoinLANSession(int32 PlayerNum, FNamedOnlineSession* Session, const FOnlineSession* SearchSession)
{
	LogSteamCoreVerbose("");
	uint32 Result = ONLINE_FAIL;
	Session->SessionState = EOnlineSessionState::Pending;

	if (Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* SearchSessionInfo = static_cast<const FOnlineSessionInfoSteamCore*>(SearchSession->SessionInfo.Get());
		FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());

		SessionInfo->m_HostAddr = SearchSessionInfo->m_HostAddr->Clone();
		SessionInfo->m_ConnectionMethod = FSteamConnectionMethod::Direct;
		Result = ONLINE_SUCCESS;
	}

	return Result;
}

bool FOnlineSessionSteamCore::FindFriendSession(int32 LocalUserNum, const FUniqueNetId& Friend)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;

	const FUniqueNetIdSteam& SteamFriendId = static_cast<const FUniqueNetIdSteam&>(Friend);

	if (!m_CurrentSessionSearch.IsValid())
	{
		FriendGameInfo_t FriendGameInfo;
		if (SteamFriends()->GetFriendGamePlayed(SteamFriendId, &FriendGameInfo))
		{
			if (FriendGameInfo.m_gameID.AppID() == m_SteamSubsystem->GetSteamAppId())
			{
				const TSharedRef<FOnlineSessionSearch> SearchSettings = MakeShareable(new FOnlineSessionSearch());
				m_CurrentSessionSearch = SearchSettings;
				m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::InProgress;

				if (FriendGameInfo.m_steamIDLobby.IsValid())
				{
					const FUniqueNetIdSteamRef LobbyId = FUniqueNetIdSteam::Create(FriendGameInfo.m_steamIDLobby);
					FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession* NewTask = new FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession(m_SteamSubsystem, *LobbyId, m_CurrentSessionSearch, LocalUserNum, OnFindFriendSessionCompleteDelegates[LocalUserNum]);
					m_SteamSubsystem->QueueAsyncTask(NewTask);
					bSuccess = true;
				}
				else
				{
					const TSharedRef<FInternetAddr> IpAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
					IpAddr->SetIp(FriendGameInfo.m_unGameIP);
					IpAddr->SetPort(FriendGameInfo.m_usGamePort);
					m_CurrentSessionSearch->QuerySettings.Set(FName(SEARCH_STEAM_HOSTIP), IpAddr->ToString(true), EOnlineComparisonOp::Equals);

					FOnlineAsyncTaskSteamCoreFindServerForFriendSession* NewTask = new FOnlineAsyncTaskSteamCoreFindServerForFriendSession(m_SteamSubsystem, m_CurrentSessionSearch, LocalUserNum, OnFindFriendSessionCompleteDelegates[LocalUserNum]);
					m_SteamSubsystem->QueueAsyncTask(NewTask);
					bSuccess = true;
				}
			}
		}
	}
	else
	{
		LogSteamCoreWarn("Ignoring friend search request while another search is pending");
	}

	if (!bSuccess)
	{
		const TArray<FOnlineSessionSearchResult> EmptyResult;
		TriggerOnFindFriendSessionCompleteDelegates(LocalUserNum, bSuccess, EmptyResult);
	}

	return bSuccess;
}

bool FOnlineSessionSteamCore::FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend)
{
	return FindFriendSession(0, Friend);
}

bool FOnlineSessionSteamCore::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& FriendList)
{
	LogSteamCoreVerbose("FOnlineSessionSteamCore::FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<FUniqueNetIdRef>& FriendList) - not implemented");
	const TArray<FOnlineSessionSearchResult> EmptyResult;
	TriggerOnFindFriendSessionCompleteDelegates(0, false, EmptyResult);
	return false;
}

bool FOnlineSessionSteamCore::PingSearchResults(const FOnlineSessionSearchResult& SearchResult)
{
	return false;
}

void FOnlineSessionSteamCore::CheckPendingSessionInvite()
{
	LogSteamCoreVerbose("");
	const TCHAR* CmdLine = FCommandLine::Get();
	const FString CmdLineStr(CmdLine);

	const FString LobbyConnectCmd = TEXT("+connect_lobby");
	int32 ConnectIdx = CmdLineStr.Find(LobbyConnectCmd, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	if (ConnectIdx != INDEX_NONE)
	{
		const TCHAR* Str = CmdLine + ConnectIdx + LobbyConnectCmd.Len();
		const FString LobbyIdStr = FParse::Token(Str, 0);
		int64 LobbyId = FCString::Strtoui64(*LobbyIdStr, nullptr, 10);
		if (LobbyId > 0)
		{
			m_PendingInvite.m_PendingInviteType = ESteamSession::LobbySession;
			m_PendingInvite.m_LobbyId = FUniqueNetIdSteam::Create(LobbyId);
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
				m_PendingInvite.m_PendingInviteType = ESteamSession::AdvertisedSessionClient;
				m_PendingInvite.m_ServerIp = FString::Printf(TEXT("-SteamConnectIP=%s"), *ServerIpAddrStr);
			}
		}
	}
}

bool FOnlineSessionSteamCore::SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend)
{
	TArray<FUniqueNetIdRef> Friends;
	Friends.Add(Friend.AsShared());
	return SendSessionInviteToFriends(LocalUserNum, SessionName, Friends);
}

bool FOnlineSessionSteamCore::SendSessionInviteToFriend(const FUniqueNetId& LocalUserId, FName SessionName, const FUniqueNetId& Friend)
{
	return SendSessionInviteToFriend(0, SessionName, Friend);
}

bool FOnlineSessionSteamCore::SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<FUniqueNetIdRef>& Friends)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session && Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		if (SessionInfo->m_SessionType == ESteamSession::LobbySession && SessionInfo->m_SessionId->IsValid())
		{
			for (int32 FriendIdx = 0; FriendIdx < Friends.Num(); FriendIdx++)
			{
				const FUniqueNetIdSteam& FriendId = FUniqueNetIdSteam::Cast(*Friends[FriendIdx]);

				if (SteamMatchmaking()->InviteUserToLobby(*SessionInfo->m_SessionId, FriendId))
				{
					bSuccess = true;
				}
				else
				{
					LogSteamCoreWarn("Error inviting %s to session %s, not connected to Steam", *FriendId.ToDebugString(), *SessionName.ToString());
				}
			}
		}
		else if (SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionHost || SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionClient)
		{
			const FString ConnectionURL = GetSteamConnectionString(SessionName);

			for (int32 FriendIdx = 0; FriendIdx < Friends.Num(); FriendIdx++)
			{
				FUniqueNetIdSteam& FriendId = (FUniqueNetIdSteam&)(Friends[FriendIdx].Get());

				if (SteamFriends()->InviteUserToGame(FriendId, TCHAR_TO_UTF8(*ConnectionURL)))
				{
					LogSteamCoreVerbose("Inviting %s to session %s with %s", *FriendId.ToDebugString(), *SessionName.ToString(), *ConnectionURL);
					bSuccess = true;
				}
				else
				{
					LogSteamCoreWarn("Error inviting %s to session %s", *FriendId.ToDebugString(), *SessionName.ToString());
				}
			}
		}
		else
		{
			LogSteamCoreWarn("Invalid session info for invite %s", *SessionName.ToString());
		}
	}
	else
	{
		LogSteamCoreWarn("Missing or invalid session %s for invite request", *SessionName.ToString());
	}

	return bSuccess;
}

bool FOnlineSessionSteamCore::SendSessionInviteToFriends(const FUniqueNetId& LocalUserId, FName SessionName, const TArray<FUniqueNetIdRef>& Friends)
{
	return SendSessionInviteToFriends(0, SessionName, Friends);
}

FString FOnlineSessionSteamCore::GetSteamConnectionString(FName SessionName)
{
	FString ConnectionString;

	const FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session && Session->SessionInfo.IsValid())
	{
		const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
		if (SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionHost || SessionInfo->m_SessionType == ESteamSession::AdvertisedSessionClient)
		{
			ConnectionString = FString::Printf(TEXT("-SteamConnectIP=%s"), *SessionInfo->m_HostAddr->ToString(true));
		}
	}

	return ConnectionString;
}

/** Get a resolved connection string from a session info */
static bool GetConnectStringFromSessionInfo(TSharedPtr<FOnlineSessionInfoSteamCore>& SessionInfo, FString& ConnectInfo, int32 PortOverride = 0)
{
	LogSteamCoreVerbose("GetConnectStringFromSessionInfo");
	
	bool bSuccess = false;

	if (SessionInfo.IsValid())
	{
		const bool bP2PDataValid = (SessionInfo->m_SteamP2PAddr.IsValid() && SessionInfo->m_SteamP2PAddr->IsValid());
		const bool bHostDataValid = (SessionInfo->m_HostAddr.IsValid() && SessionInfo->m_HostAddr->IsValid());

		LogSteamCoreVerbose("SessionInfo is Valid. bP2PDataValid: (%d), bHostDataValid: (%d)", bP2PDataValid, bHostDataValid);
		
		// If we have host data, attempt to use it.
		if (bHostDataValid && SessionInfo->m_ConnectionMethod == FSteamConnectionMethod::Direct)
		{
			LogSteamCoreVerbose("Using Host Data for Connection Serialization");

			int32 HostPort = SessionInfo->m_HostAddr->GetPort();
			if (PortOverride > 0)
			{
				HostPort = PortOverride;
			}

			ConnectInfo = FString::Printf(TEXT("%s:%d"), *SessionInfo->m_HostAddr->ToString(false), HostPort);
			bSuccess = true;
		}
		else if (bP2PDataValid)
		{
			LogSteamCoreVerbose("Using P2P Data for Connection Serialization");

			int32 SteamPort = SessionInfo->m_SteamP2PAddr->GetPort();
			if (PortOverride > 0)
			{
				SteamPort = PortOverride;
			}

			ConnectInfo = FString::Printf(STEAM_URL_PREFIX TEXT("%s:%d"), *SessionInfo->m_SteamP2PAddr->ToString(false), SteamPort);
			bSuccess = true;
		}
		else
		{
			LogSteamCoreWarn("Steam could not resolve session info! ValidP2P[%d] ValidHost[%d] ConnectionMethod[%s]", bP2PDataValid, bHostDataValid, *SteamCore::LexToString(SessionInfo->m_ConnectionMethod));
			return false;
		}
	}

	return bSuccess;
}

bool FOnlineSessionSteamCore::GetResolvedConnectString(FName SessionName, FString& ConnectInfo, FName PortType)
{
	bool bSuccess = false;
	if (const FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		TSharedPtr<FOnlineSessionInfoSteamCore> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoSteamCore>(Session->SessionInfo);
		if (PortType == NAME_BeaconPort)
		{
			const int32 BeaconListenPort = GetBeaconPortFromSessionSettings(Session->SessionSettings);
			bSuccess = GetConnectStringFromSessionInfo(SessionInfo, ConnectInfo, BeaconListenPort);
		}
		else if (PortType == NAME_GamePort)
		{
			bSuccess = GetConnectStringFromSessionInfo(SessionInfo, ConnectInfo);
		}

		if (!bSuccess)
		{
			LogSteamCoreWarn("Invalid session info for session %s in GetResolvedConnectString()", *SessionName.ToString());
		}
	}
	else
	{
		LogSteamCoreWarn("Unknown session name (%s) specified to GetResolvedConnectString()", *SessionName.ToString());
	}

	return bSuccess;
}

bool FOnlineSessionSteamCore::GetResolvedConnectString(const FOnlineSessionSearchResult& SearchResult, const FName PortType, FString& ConnectInfo)
{
	bool bSuccess = false;
	if (SearchResult.Session.SessionInfo.IsValid())
	{
		TSharedPtr<FOnlineSessionInfoSteamCore> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoSteamCore>(SearchResult.Session.SessionInfo);

		if (PortType == NAME_BeaconPort)
		{
			const int32 BeaconListenPort = GetBeaconPortFromSessionSettings(SearchResult.Session.SessionSettings);
			bSuccess = GetConnectStringFromSessionInfo(SessionInfo, ConnectInfo, BeaconListenPort);
		}
		else if (PortType == NAME_GamePort)
		{
			bSuccess = GetConnectStringFromSessionInfo(SessionInfo, ConnectInfo);
		}
	}

	if (!bSuccess || ConnectInfo.IsEmpty())
	{
		LogSteamCoreWarn("Invalid session info in search result to GetResolvedConnectString()");
	}

	return bSuccess;
}

FString FOnlineSessionSteamCore::GetCustomDedicatedServerName() const
{
	FString ServerName;

	if (FParse::Value(FCommandLine::Get(), TEXT("-SteamServerName="), ServerName))
	{
		if (ServerName.Len() >= k_cbMaxGameServerName)
		{
			LogSteamCoreWarn("SteamServerName overflows the maximum amount of characters %d allowed, truncating.", k_cbMaxGameServerName);
			ServerName.LeftInline(k_cbMaxGameServerName - 1);
		}

		return ServerName;
	}

	return TEXT("");
}

FUniqueNetIdPtr FOnlineSessionSteamCore::CreateSessionIdFromString(const FString& SessionIdStr)
{
	if (!SessionIdStr.IsEmpty())
	{
		return FUniqueNetIdSteam::Create(SessionIdStr);
	}
	return nullptr;
}

FOnlineSessionSettings* FOnlineSessionSteamCore::GetSessionSettings(FName SessionName)
{
	FNamedOnlineSession* Session = GetNamedSession(SessionName);
	if (Session)
	{
		return &Session->SessionSettings;
	}
	return nullptr;
}

void FOnlineSessionSteamCore::RegisterVoice(const FUniqueNetId& PlayerId) const
{
	LogSteamCoreVerbose("");
	if (!m_SteamSubsystem->IsDedicated())
	{
		if (PlayerId.IsValid())
		{
			const IOnlineVoicePtr VoiceInt = m_SteamSubsystem->GetVoiceInterface();
			if (VoiceInt.IsValid())
			{
				if (!m_SteamSubsystem->IsLocalPlayer(PlayerId))
				{
					VoiceInt->RegisterRemoteTalker(PlayerId);
				}
				else
				{
					VoiceInt->ProcessMuteChangeNotification();
				}
			}
		}
	}
}

void FOnlineSessionSteamCore::UnregisterVoice(const FUniqueNetId& PlayerId) const
{
	LogSteamCoreVerbose("");
	if (!m_SteamSubsystem->IsDedicated())
	{
		const IOnlineVoicePtr VoiceInt = m_SteamSubsystem->GetVoiceInterface();
		if (VoiceInt.IsValid())
		{
			if (PlayerId.IsValid() && !m_SteamSubsystem->IsLocalPlayer(PlayerId))
			{
				VoiceInt->UnregisterRemoteTalker(PlayerId);
			}
		}
	}
}

bool FOnlineSessionSteamCore::RegisterPlayer(FName SessionName, const FUniqueNetId& PlayerId, bool bWasInvited)
{
	TArray<FUniqueNetIdRef> Players;
	Players.Add(PlayerId.AsShared());
	return RegisterPlayers(SessionName, Players, bWasInvited);
}

bool FOnlineSessionSteamCore::RegisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players, bool bWasInvited)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;
	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (Session->SessionInfo.IsValid())
		{
			FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());

			ISteamFriends* SteamFriendsPtr = SteamFriends();
			for (int32 PlayerIdx = 0; PlayerIdx < Players.Num(); PlayerIdx++)
			{
				const FUniqueNetIdRef& PlayerId = Players[PlayerIdx];
				const FUniqueNetIdSteam& SteamId = static_cast<const FUniqueNetIdSteam&>(*PlayerId);

				const FUniqueNetIdMatcher PlayerMatch(SteamId);
				if (Session->RegisteredPlayers.IndexOfByPredicate(PlayerMatch) == INDEX_NONE)
				{
					Session->RegisteredPlayers.Add(PlayerId);

					if (!m_SteamSubsystem->IsLocalPlayer(SteamId))
					{
						if (SteamFriendsPtr)
						{
							SteamFriendsPtr->RequestUserInformation(SteamId, true);
						}
					}
				}
				else
				{
					LogSteamCoreVerbose("Player %s already registered in session %s", *Players[PlayerIdx]->ToDebugString(), *SessionName.ToString());
				}

				RegisterVoice(SteamId);
			}

			bSuccess = true;
		}
		else
		{
			LogSteamCoreWarn("No session info to join for session (%s)", *SessionName.ToString());
		}
	}
	else
	{
		LogSteamCoreWarn("No game present to join for session (%s)", *SessionName.ToString());
	}

	TriggerOnRegisterPlayersCompleteDelegates(SessionName, Players, bSuccess);
	return bSuccess;
}

void FOnlineSessionSteamCore::RegisterLocalPlayers(FNamedOnlineSession* Session) const
{
	LogSteamCoreVerbose("");
	if (!m_SteamSubsystem->IsDedicated())
	{
		const IOnlineVoicePtr VoiceInt = m_SteamSubsystem->GetVoiceInterface();
		LogSteamCoreVerbose("VoiceInt IsValid: %d", VoiceInt.IsValid());
		
		if (VoiceInt.IsValid())
		{
			for (int32 Index = 0; Index < MAX_LOCAL_PLAYERS; Index++)
			{
				VoiceInt->RegisterLocalTalker(Index);
			}
		}
	}
}

bool FOnlineSessionSteamCore::UnregisterPlayer(FName SessionName, const FUniqueNetId& PlayerId)
{
	TArray<FUniqueNetIdRef> Players;
	Players.Add(PlayerId.AsShared());
	return UnregisterPlayers(SessionName, Players);
}

bool FOnlineSessionSteamCore::UnregisterPlayers(FName SessionName, const TArray<FUniqueNetIdRef>& Players)
{
	LogSteamCoreVerbose("");
	bool bSuccess = false;

	if (FNamedOnlineSession* Session = GetNamedSession(SessionName))
	{
		if (Session->SessionInfo.IsValid())
		{
			FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());

			for (int32 PlayerIdx = 0; PlayerIdx < Players.Num(); PlayerIdx++)
			{
				const FUniqueNetIdRef& PlayerId = Players[PlayerIdx];

				const FUniqueNetIdMatcher PlayerMatch(*PlayerId);
				const int32 RegistrantIndex = Session->RegisteredPlayers.IndexOfByPredicate(PlayerMatch);
				if (RegistrantIndex != INDEX_NONE)
				{
					Session->RegisteredPlayers.RemoveAtSwap(RegistrantIndex);
					UnregisterVoice(*PlayerId);
				}
				else
				{
					LogSteamCoreWarn("Player %s is not part of session (%s)", *PlayerId->ToDebugString(), *SessionName.ToString());
				}
			}

			bSuccess = true;
		}
		else
		{
			LogSteamCoreWarn("No session info to leave for session (%s)", *SessionName.ToString());
		}
	}
	else
	{
		LogSteamCoreWarn("No game present to leave for session (%s)", *SessionName.ToString());
	}

	TriggerOnUnregisterPlayersCompleteDelegates(SessionName, Players, bSuccess);
	return bSuccess;
}

void FOnlineSessionSteamCore::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Session_Interface);
	TickLanTasks(DeltaTime);
	TickPendingInvites(DeltaTime);
}

FString UOnlineSessionInterfaceSteamCoreUtils::GetLastJoinedLobbyId()
{
	FString Result;
	
	const FOnlineSubsystemSteamCore* Subsystem = static_cast<const FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (Subsystem)
	{
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

		if (SessionInt)
		{
			TArray<FUniqueNetIdSteamRef> JoinedLobbyList = SessionInt->GetJoinedLobbyList();

			if (JoinedLobbyList.IsValidIndex(0))
			{
				Result = JoinedLobbyList[0]->ToString();
			}
		}
	}

	return Result;
}

TArray<FString> UOnlineSessionInterfaceSteamCoreUtils::GetJoinedLobbyIds()
{
	TArray<FString> Result;
	
	const FOnlineSubsystemSteamCore* Subsystem = static_cast<const FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (Subsystem)
	{
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

		if (SessionInt)
		{
			TArray<FUniqueNetIdSteamRef> JoinedLobbyList = SessionInt->GetJoinedLobbyList();

			for (const auto& Element : JoinedLobbyList)
			{
				Result.Add(Element->ToString());
			}
		}
	}

	return Result;
}

void FOnlineSessionSteamCore::TickLanTasks(float DeltaTime) const
{
	if (m_LANSession != nullptr && m_LANSession->GetBeaconState() > ELanBeaconState::NotUsingLanBeacon)
	{
		m_LANSession->Tick(DeltaTime);
	}
}

void FOnlineSessionSteamCore::TickPendingInvites(float DeltaTime)
{
	if (m_PendingInvite.m_PendingInviteType != ESteamSession::None)
	{
		if (OnSessionUserInviteAcceptedDelegates.IsBound())
		{
			FOnlineAsyncItem* NewEvent = nullptr;
			if (m_PendingInvite.m_PendingInviteType == ESteamSession::LobbySession)
			{
				NewEvent = new FOnlineAsyncEventSteamCoreLobbyInviteAccepted(m_SteamSubsystem, *FUniqueNetIdSteam::EmptyId(), *m_PendingInvite.m_LobbyId);
			}
			else
			{
				NewEvent = new FOnlineAsyncEventSteamCoreInviteAccepted(m_SteamSubsystem, *FUniqueNetIdSteam::EmptyId(), m_PendingInvite.m_ServerIp);
			}

			if (NewEvent)
			{
				LogSteamCoreVerbose("%s", *NewEvent->ToString());
				m_SteamSubsystem->QueueAsyncOutgoingItem(NewEvent);
			}

			m_PendingInvite.m_PendingInviteType = ESteamSession::None;
		}
	}
}

void FOnlineSessionSteamCore::AppendSessionToPacket(FNboSerializeToBufferSteamCore& Packet, FOnlineSession* Session) const
{
	LogSteamCoreVerbose("");
	((FNboSerializeToBuffer&)Packet) << StaticCastSharedPtr<const FUniqueNetIdSteam>(Session->OwningUserId)->m_UniqueNetId
		<< Session->OwningUserName
		<< Session->NumOpenPrivateConnections
		<< Session->NumOpenPublicConnections;

	Packet << *StaticCastSharedPtr<FOnlineSessionInfoSteamCore>(Session->SessionInfo);
	AppendSessionSettingsToPacket(Packet, &Session->SessionSettings);
}

void FOnlineSessionSteamCore::AppendSessionSettingsToPacket(FNboSerializeToBufferSteamCore& Packet, FOnlineSessionSettings* SessionSettings)
{
	LogSteamCoreVerbose("");
#if DEBUG_LAN_BEACON
	LogSteamCoreVerbose("Sending session settings to client");
#endif

	((FNboSerializeToBuffer&)Packet) << SessionSettings->NumPublicConnections
		<< SessionSettings->NumPrivateConnections
		<< static_cast<uint8>(SessionSettings->bShouldAdvertise)
		<< static_cast<uint8>(SessionSettings->bIsLANMatch)
		<< static_cast<uint8>(SessionSettings->bIsDedicated)
		<< static_cast<uint8>(SessionSettings->bUsesStats)
		<< static_cast<uint8>(SessionSettings->bAllowJoinInProgress)
		<< static_cast<uint8>(SessionSettings->bAllowInvites)
		<< static_cast<uint8>(SessionSettings->bUsesPresence)
		<< static_cast<uint8>(SessionSettings->bAllowJoinViaPresence)
		<< static_cast<uint8>(SessionSettings->bAllowJoinViaPresenceFriendsOnly)
		<< static_cast<uint8>(SessionSettings->bAntiCheatProtected)
		<< SessionSettings->BuildUniqueId;

	int32 NumAdvertisedProperties = 0;
	for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
	{
		const FOnlineSessionSetting& Setting = It.Value();
		if (Setting.AdvertisementType >= EOnlineDataAdvertisementType::ViaOnlineService)
		{
			NumAdvertisedProperties++;
		}
	}

	((FNboSerializeToBuffer&)Packet) << (int32)NumAdvertisedProperties;
	for (FSessionSettings::TConstIterator It(SessionSettings->Settings); It; ++It)
	{
		const FOnlineSessionSetting& Setting = It.Value();
		if (Setting.AdvertisementType >= EOnlineDataAdvertisementType::ViaOnlineService)
		{
			((FNboSerializeToBuffer&)Packet) << It.Key();
			Packet << Setting;
#if DEBUG_LAN_BEACON
			LogSteamCoreVerbose("%s", *Setting.ToString());
#endif
		}
	}
}

void FOnlineSessionSteamCore::OnValidQueryPacketReceived(uint8* PacketData, int32 PacketLength, uint64 ClientNonce)
{
	LogSteamCoreVerbose("");
	FScopeLock ScopeLock(&m_SessionLock);
	for (int32 SessionIndex = 0; SessionIndex < m_Sessions.Num(); SessionIndex++)
	{
		FNamedOnlineSession& Session = m_Sessions[SessionIndex];

		const FOnlineSessionSettings& Settings = Session.SessionSettings;

		const bool bIsMatchInProgress = Session.SessionState == EOnlineSessionState::InProgress;

		const bool bIsMatchJoinable = Settings.bIsLANMatch &&
			(!bIsMatchInProgress || Settings.bAllowJoinInProgress) &&
			Settings.NumPublicConnections > 0;

		if (bIsMatchJoinable)
		{
			FNboSerializeToBufferSteamCore Packet(LAN_BEACON_MAX_PACKET_SIZE);
			m_LANSession->CreateHostResponsePacket(Packet, ClientNonce);

			AppendSessionToPacket(Packet, &Session);

			m_LANSession->BroadcastPacket(Packet, Packet.GetByteCount());
		}
	}
}

void FOnlineSessionSteamCore::ReadSessionFromPacket(FNboSerializeFromBufferSteamCore& Packet, FOnlineSession* Session) const
{
	LogSteamCoreVerbose("");
#if DEBUG_LAN_BEACON
	LogSteamCoreVerbose("Reading session information from server");
#endif

	uint64 OwningUserId;
	Packet >> OwningUserId
		>> Session->OwningUserName
		>> Session->NumOpenPrivateConnections
		>> Session->NumOpenPublicConnections;

	Session->OwningUserId = FUniqueNetIdSteam::Create(OwningUserId);

	FOnlineSessionInfoSteamCore* SteamSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::LANSession);
	SteamSessionInfo->m_HostAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Packet >> *SteamSessionInfo;
	Session->SessionInfo = MakeShareable(SteamSessionInfo);

	ReadSettingsFromPacket(Packet, Session->SessionSettings);
}

void FOnlineSessionSteamCore::ReadSettingsFromPacket(FNboSerializeFromBufferSteamCore& Packet, FOnlineSessionSettings& SessionSettings)
{
	LogSteamCoreVerbose("");
#if DEBUG_LAN_BEACON
	LogSteamCoreVerbose("Reading game settings from server");
#endif

	SessionSettings.Settings.Empty();

	Packet >> SessionSettings.NumPublicConnections
		>> SessionSettings.NumPrivateConnections;
	uint8 Read = 0;
	Packet >> Read;
	SessionSettings.bShouldAdvertise = !!Read;
	Packet >> Read;
	SessionSettings.bIsLANMatch = !!Read;
	Packet >> Read;
	SessionSettings.bIsDedicated = !!Read;
	Packet >> Read;
	SessionSettings.bUsesStats = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinInProgress = !!Read;
	Packet >> Read;
	SessionSettings.bAllowInvites = !!Read;
	Packet >> Read;
	SessionSettings.bUsesPresence = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinViaPresence = !!Read;
	Packet >> Read;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = !!Read;
	Packet >> Read;
	SessionSettings.bAntiCheatProtected = !!Read;

	Packet >> SessionSettings.BuildUniqueId;

	int32 NumAdvertisedProperties = 0;
	Packet >> NumAdvertisedProperties;
	if (Packet.HasOverflow() == false)
	{
		FName Key;
		for (int32 Index = 0;
		     Index < NumAdvertisedProperties && Packet.HasOverflow() == false;
		     Index++)
		{
			FOnlineSessionSetting Setting;
			Packet >> Key;
			Packet >> Setting;
			SessionSettings.Set(Key, Setting);

#if DEBUG_LAN_BEACON
			LogSteamCoreVerbose("%s", *Setting->ToString());
#endif
		}
	}

	if (Packet.HasOverflow())
	{
		SessionSettings.Settings.Empty();
		LogSteamCoreVerbose("Packet overflow detected in ReadGameSettingsFromPacket()");
	}
}

void FOnlineSessionSteamCore::OnValidResponsePacketReceived(uint8* PacketData, int32 PacketLength)
{
	LogSteamCoreVerbose("");
	FOnlineSessionSettings NewServer;
	if (m_CurrentSessionSearch.IsValid())
	{
		FOnlineSessionSearchResult* NewResult = new(m_CurrentSessionSearch->SearchResults) FOnlineSessionSearchResult();
		FOnlineSession* NewSession = &NewResult->Session;

		FNboSerializeFromBufferSteamCore Packet(PacketData, PacketLength);

		ReadSessionFromPacket(Packet, NewSession);
	}
	else
	{
		LogSteamCoreWarn("Failed to create new online game settings object");
	}
}

void FOnlineSessionSteamCore::OnLANSearchTimeout()
{
	LogSteamCoreVerbose("");

	bool bWasHosting = false;
	{
		FScopeLock ScopeLock(&m_SessionLock);
		for (int32 SessionIdx = 0; SessionIdx < m_Sessions.Num(); SessionIdx++)
		{
			const FNamedOnlineSession& Session = m_Sessions[SessionIdx];
			if (Session.SessionSettings.bShouldAdvertise &&
				Session.SessionSettings.bIsLANMatch &&
				m_SteamSubsystem->IsServer())
			{
				bWasHosting = true;
				break;
			}
		}
	}

	if (bWasHosting)
	{
		FOnValidQueryPacketDelegate QueryPacketDelegate = FOnValidQueryPacketDelegate::CreateRaw(this, &FOnlineSessionSteamCore::OnValidQueryPacketReceived);
		if (m_LANSession->Host(QueryPacketDelegate))
		{
			LogSteamCoreWarn("Failed to restart hosted LAN session after search completion");
		}
	}
	else
	{
		m_LANSession->StopLANSession();
	}

	if (m_CurrentSessionSearch.IsValid())
	{
		if (m_CurrentSessionSearch->SearchResults.Num() > 0)
		{
			m_CurrentSessionSearch->SortSearchResults();
		}
		m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::Done;

		m_CurrentSessionSearch = nullptr;
	}

	TriggerOnFindSessionsCompleteDelegates(true);
}

void FOnlineSessionSteamCore::SyncLobbies()
{
	LogSteamCoreVerbose("Member of %d lobbies", m_JoinedLobbyList.Num());
	TArray<FUniqueNetIdSteamRef> LobbiesToRemove = m_JoinedLobbyList;

	{
		FScopeLock ScopeLock(&m_SessionLock);
		for (int32 SessionIdx = 0; SessionIdx < m_Sessions.Num(); SessionIdx++)
		{
			const FNamedOnlineSession& Session = m_Sessions[SessionIdx];
			const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session.SessionInfo.Get());
			if (SessionInfo->m_SessionType == ESteamSession::LobbySession && SessionInfo->m_SessionId->IsValid())
			{
				LobbiesToRemove.RemoveSingleSwap(SessionInfo->m_SessionId);
			}
		}
	}

	for (int32 LobbyIdx = 0; LobbyIdx < LobbiesToRemove.Num(); LobbyIdx++)
	{
		const FUniqueNetIdSteam& LobbyId = *LobbiesToRemove[LobbyIdx];
		LogSteamCoreVerbose("Lobby %s out of sync, removing...", *LobbyId.ToDebugString());
		FOnlineAsyncTaskSteamCoreLeaveLobby* NewTask = new FOnlineAsyncTaskSteamCoreLeaveLobby(m_SteamSubsystem, TEXT("OUTOFSYNC"), LobbyId);
		m_SteamSubsystem->QueueAsyncTask(NewTask);
	}
}

int32 FOnlineSessionSteamCore::GetNumSessions()
{
	FScopeLock ScopeLock(&m_SessionLock);
	return m_Sessions.Num();
}

void FOnlineSessionSteamCore::DumpSessionState()
{
	FScopeLock ScopeLock(&m_SessionLock);

	LogSteamCoreVerbose("Member of %d lobbies", m_JoinedLobbyList.Num());
	TArray<FUniqueNetIdSteamRef> OutOfSyncLobbies = m_JoinedLobbyList;
	for (int32 SessionIdx = 0; SessionIdx < m_Sessions.Num(); SessionIdx++)
	{
		const FNamedOnlineSession& Session = m_Sessions[SessionIdx];
		const FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session.SessionInfo.Get());
		if (SessionInfo->m_SessionType == ESteamSession::LobbySession && SessionInfo->m_SessionId->IsValid())
		{
			OutOfSyncLobbies.RemoveSingleSwap(SessionInfo->m_SessionId);
		}
	}

	if (OutOfSyncLobbies.Num() > 0)
	{
		LogSteamCoreVerbose("Out of sync lobbies: %d", OutOfSyncLobbies.Num());
		for (int32 LobbyIdx = 0; LobbyIdx < OutOfSyncLobbies.Num(); LobbyIdx++)
		{
			LogSteamCoreVerbose("%s", *OutOfSyncLobbies[LobbyIdx]->ToDebugString());
		}
	}

	for (int32 SessionIdx = 0; SessionIdx < m_Sessions.Num(); SessionIdx++)
	{
		DumpNamedSession(&m_Sessions[SessionIdx]);
	}
}

void FOnlineSessionSteamCore::RegisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnRegisterLocalPlayerCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	Delegate.ExecuteIfBound(PlayerId, EOnJoinSessionCompleteResult::Success);
}

void FOnlineSessionSteamCore::UnregisterLocalPlayer(const FUniqueNetId& PlayerId, FName SessionName, const FOnUnregisterLocalPlayerCompleteDelegate& Delegate)
{
	LogSteamCoreVerbose("");
	Delegate.ExecuteIfBound(PlayerId, true);
}

namespace SteamCore
{
	FString LexToString(const FSteamConnectionMethod Method)
	{
		switch (Method)
		{
		default:
		case FSteamConnectionMethod::None:
			return TEXT("None");
		case FSteamConnectionMethod::Direct:
			return TEXT("Direct");
		case FSteamConnectionMethod::P2P:
			return TEXT("P2P");
		case FSteamConnectionMethod::PartnerHosted:
			return TEXT("PartnerHosted");
		}
	}

	FSteamConnectionMethod ToConnectionMethod(const FString& InString)
	{
		if (InString == TEXT("Direct"))
		{
			return FSteamConnectionMethod::Direct;
		}
		else if (InString == TEXT("P2P"))
		{
			return FSteamConnectionMethod::P2P;
		}
		else if (InString == TEXT("PartnerHosted"))
		{
			return FSteamConnectionMethod::PartnerHosted;
		}
		else
		{
			return FSteamConnectionMethod::None;
		}
	}
}
#endif