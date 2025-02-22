/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Sockets/SocketSubsystemSteamCore.h"
#include "SteamCoreSharedTypes.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE
FSocketSubsystemSteamCore* FSocketSubsystemSteamCore::m_SocketSingleton = nullptr;

FSocketSubsystemSteamCore* FSocketSubsystemSteamCore::Create()
{
	LogSteamCoreVerbose("");
	if (m_SocketSingleton == nullptr)
	{
		m_SocketSingleton = new FSocketSubsystemSteamCore();
	}

	return m_SocketSingleton;
} 

void FSocketSubsystemSteamCore::Destroy()
{
	LogSteamCoreVerbose("");
	if (m_SocketSingleton != nullptr)
	{
		m_SocketSingleton->Shutdown();
		delete m_SocketSingleton;
		m_SocketSingleton = nullptr;
	}
}

bool FSocketSubsystemSteamCore::Init(FString& Error)
{
	LogSteamCoreVerbose("");
	if (GConfig)
	{
		if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bAllowP2PPacketRelay"), m_bAllowP2PPacketRelay, GEngineIni))
		{
			LogSteamCoreWarn("Missing bAllowP2PPacketRelay key in OnlineSubsystemSteamCore of DefaultEngine.ini");
		}

		if (!GConfig->GetFloat(TEXT("OnlineSubsystemSteamCore"), TEXT("P2PConnectionTimeout"), m_P2PConnectionTimeout, GEngineIni))
		{
			LogSteamCoreWarn("Missing P2PConnectionTimeout key in OnlineSubsystemSteamCore of DefaultEngine.ini");
		}

		if (!GConfig->GetDouble(TEXT("OnlineSubsystemSteamCore"), TEXT("P2PCleanupTimeout"), m_P2PCleanupTimeout, GEngineIni))
		{
			LogSteamCoreVerbose("Missing P2PCleanupTimeout key in OnlineSubsystemSteamCore of DefaultEngine.ini, using default");
		}
	}

	if (SteamNetworking())
	{
		SteamNetworking()->AllowP2PPacketRelay(m_bAllowP2PPacketRelay);
	}

	if (SteamGameServerNetworking())
	{
		SteamGameServerNetworking()->AllowP2PPacketRelay(m_bAllowP2PPacketRelay);
	}

	return true;
}

void FSocketSubsystemSteamCore::Shutdown()
{
	LogSteamCoreVerbose("");
	for (int32 ConnIdx = m_SteamConnections.Num() - 1; ConnIdx >= 0; ConnIdx--)
	{
		if (m_SteamConnections[ConnIdx].IsValid())
		{
			USteamCoreNetConnection* SteamConn = CastChecked<USteamCoreNetConnection>(m_SteamConnections[ConnIdx].Get());
			UnregisterConnection(SteamConn);
		}
	}

	LogSteamCoreVerbose("Shutting down SteamNet connections");

	m_DeadConnections.Empty();

	for (auto SessionIds : m_AcceptedConnections)
	{
		P2PRemove(FUniqueNetIdSteam::Cast(*SessionIds.Key), -1);
	}

	CleanupDeadConnections(true);

	TArray<FSocketSteamCore*> TempArray = m_SteamSockets;
	for (int SocketIdx = 0; SocketIdx < TempArray.Num(); SocketIdx++)
	{
		DestroySocket(TempArray[SocketIdx]);
	}

	m_SteamSockets.Empty();
	m_SteamConnections.Empty();
	m_AcceptedConnections.Empty();
	m_DeadConnections.Empty();
}

FSocket* FSocketSubsystemSteamCore::CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType)
{
	LogSteamCoreVerbose("");
	FSocket* NewSocket = nullptr;
	if (SocketType == FName("SteamClientSocket"))
	{
		ISteamUser* SteamUserPtr = SteamUser();
		if (SteamUserPtr != nullptr)
		{
			const FUniqueNetIdSteamRef ClientId = FUniqueNetIdSteam::Create(SteamUserPtr->GetSteamID());
			NewSocket = new FSocketSteamCore(SteamNetworking(), *ClientId, SocketDescription, FNetworkProtocolTypes::SteamCore);

			if (NewSocket)
			{
				AddSocket(static_cast<FSocketSteamCore*>(NewSocket));
			}
		}
	}
	else if (SocketType == FName("SteamServerSocket"))
	{
		const IOnlineSubsystem* SteamSubsystem = IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM);
		const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(SteamSubsystem->GetSessionInterface());
		if (SessionInt.IsValid())
		{
			if (SessionInt->m_bSteamworksGameServerConnected && SessionInt->m_GameServerSteamId->IsValid() && SessionInt->m_bPolicyResponseReceived)
			{
				NewSocket = new FSocketSteamCore(SteamGameServerNetworking(), *SessionInt->m_GameServerSteamId, SocketDescription, FNetworkProtocolTypes::SteamCore);
			}
			else
			{
				NewSocket = new FSocketSteamCore(SteamGameServerNetworking(), *FUniqueNetIdSteam::EmptyId(), SocketDescription, FNetworkProtocolTypes::SteamCore);
			}

			if (NewSocket)
			{
				AddSocket(static_cast<FSocketSteamCore*>(NewSocket));
			}
		}
	}
	else
	{
		if (ISocketSubsystem* PlatformSocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
		{
			NewSocket = PlatformSocketSub->CreateSocket(SocketType, SocketDescription, ProtocolType);
		}
	}

	if (!NewSocket)
	{
		LogSteamCoreWarn("Failed to create socket %s [%s]", *SocketType.ToString(), *SocketDescription);
	}

	return NewSocket;
}

void FSocketSubsystemSteamCore::DestroySocket(FSocket* Socket)
{
	LogSteamCoreVerbose("");
	RemoveSocket(static_cast<FSocketSteamCore*>(Socket));
	delete Socket;
}

void FSocketSubsystemSteamCore::FixupSockets(const FUniqueNetIdSteam& GameServerId)
{
	LogSteamCoreVerbose("");
	for (int32 SockIdx = 0; SockIdx < m_SteamSockets.Num(); SockIdx++)
	{
		FSocketSteamCore* Socket = m_SteamSockets[SockIdx];
		if (Socket->m_SteamNetworkingPtr == SteamGameServerNetworking() && !Socket->m_LocalSteamId->IsValid())
		{
			Socket->m_LocalSteamId = GameServerId.AsShared();
		}
	}
}

void FSocketSubsystemSteamCore::RegisterConnection(USteamCoreNetConnection* Connection)
{
	LogSteamCoreVerbose("");
	check(!Connection->m_bIsPassthrough);

	const FWeakObjectPtr ObjectPtr = Connection;
	m_SteamConnections.Add(ObjectPtr);

#if UE_VERSION_OLDER_THAN(5,1,0)
	if (FSocket* CurSocket = Connection->Socket)
#else
	if (FSocket* CurSocket = Connection->GetSocket())
#endif
	{
		const TSharedPtr<const FInternetAddr> CurRemoteAddr = Connection->GetRemoteAddr();

		if (CurRemoteAddr.IsValid())
		{
			const FSocketSteamCore* SteamSocket = static_cast<FSocketSteamCore*>(CurSocket);
			const TSharedPtr<const FInternetAddrSteamCore> SteamAddr = StaticCastSharedPtr<const FInternetAddrSteamCore>(CurRemoteAddr);

			LogSteamCoreVerbose("Adding user %s from RegisterConnection", *SteamAddr->ToString(true));

			P2PTouch(SteamSocket->m_SteamNetworkingPtr, *SteamAddr->m_SteamId, SteamAddr->m_SteamChannel);
		}
	}
}

void FSocketSubsystemSteamCore::UnregisterConnection(USteamCoreNetConnection* Connection)
{
	LogSteamCoreVerbose("");
	check(!Connection->m_bIsPassthrough);

	const FWeakObjectPtr ObjectPtr = Connection;

	if (m_SteamConnections.RemoveSingleSwap(ObjectPtr) == 1 && Connection->GetRemoteAddr().IsValid())
	{
		const TSharedPtr<const FInternetAddrSteamCore> SteamAddr = StaticCastSharedPtr<const FInternetAddrSteamCore>(Connection->GetRemoteAddr());
		P2PRemove(*SteamAddr->m_SteamId, SteamAddr->m_SteamChannel);
	}
}

void FSocketSubsystemSteamCore::ConnectFailure(const FUniqueNetIdSteam& RemoteId)
{
	LogSteamCoreVerbose("");
	for (int32 ConnIdx = m_SteamConnections.Num() - 1; ConnIdx >= 0; ConnIdx--)
	{
		if (!m_SteamConnections[ConnIdx].IsValid())
		{
			m_SteamConnections.RemoveAt(ConnIdx);
		}
	}

	for (int32 ConnIdx = 0; ConnIdx < m_SteamConnections.Num(); ConnIdx++)
	{
		USteamCoreNetConnection* SteamConn = CastChecked<USteamCoreNetConnection>(m_SteamConnections[ConnIdx].Get());
		if (SteamConn)
		{
			const TSharedPtr<const FInternetAddrSteamCore> RemoteAddrSteam = StaticCastSharedPtr<const FInternetAddrSteamCore>(SteamConn->GetRemoteAddr());
			if (*RemoteAddrSteam->m_SteamId == RemoteId)
			{
				SteamConn->Close();
			}
		}
	}

	P2PRemove(RemoteId, -1);
}

FAddressInfoResult FSocketSubsystemSteamCore::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName, EAddressInfoFlags QueryFlags, const FName ProtocolTypeName, ESocketType SocketType)
{
	LogSteamCoreVerbose("");
	FString RawAddress(HostName);

	RawAddress.RemoveFromStart(STEAM_URL_PREFIX);

	if (RawAddress.IsNumeric() && HostName != nullptr)
	{
		FAddressInfoResult SteamResult(HostName, ServiceName);

		uint64 Id = FCString::Atoi64(*RawAddress);
		if (Id != 0)
		{
			const FString PortString(ServiceName);
			SteamResult.ReturnCode = SE_NO_ERROR;
			const TSharedRef<FInternetAddrSteamCore> SteamIdAddress = StaticCastSharedRef<FInternetAddrSteamCore>(CreateInternetAddr());
			SteamIdAddress->m_SteamId = FUniqueNetIdSteam::Create(Id);
			if (PortString.IsNumeric())
			{
				SteamIdAddress->SetPort(FCString::Atoi(*PortString));
			}

			SteamResult.Results.Add(FAddressInfoResultData(SteamIdAddress, 0, FNetworkProtocolTypes::SteamCore, SOCKTYPE_Unknown));
			return SteamResult;
		}
		else
		{
			LogSteamCoreWarn("GetAddressInfo: Could not serialize %s into a SteamID, the ID was invalid.", *RawAddress);
			SteamResult.ReturnCode = SE_HOST_NOT_FOUND;
			return SteamResult;
		}
	}

	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType);
}

TSharedPtr<FInternetAddr> FSocketSubsystemSteamCore::GetAddressFromString(const FString& InAddress)
{
	FString RawAddress = InAddress;
	RawAddress.RemoveFromStart(STEAM_URL_PREFIX);

	if (RawAddress.IsNumeric())
	{
		uint64 Id = FCString::Atoi64(*RawAddress);
		if (Id != 0)
		{
			TSharedRef<FInternetAddrSteamCore> ReturnAddress = StaticCastSharedRef<FInternetAddrSteamCore>(CreateInternetAddr());
			ReturnAddress->m_SteamId = FUniqueNetIdSteam::Create(Id);
			return ReturnAddress;
		}
		else
		{
			LogSteamCoreWarn("Could not serialize %s into a SteamID, the ID was invalid.", *RawAddress);
			return nullptr;
		}
	}

	return ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressFromString(InAddress);
}

bool FSocketSubsystemSteamCore::GetHostName(FString& HostName)
{
	return false;
}

TSharedRef<FInternetAddr> FSocketSubsystemSteamCore::CreateInternetAddr()
{
	return MakeShareable(new FInternetAddrSteamCore());
}

bool FSocketSubsystemSteamCore::HasNetworkDevice()
{
	return true;
}

const TCHAR* FSocketSubsystemSteamCore::GetSocketAPIName() const
{
	return TEXT("SteamSockets");
}

ESocketErrors FSocketSubsystemSteamCore::GetLastErrorCode()
{
	return TranslateErrorCode(m_LastSocketError);
}

ESocketErrors FSocketSubsystemSteamCore::TranslateErrorCode(int32 Code)
{
	return static_cast<ESocketErrors>(m_LastSocketError);
}

bool FSocketSubsystemSteamCore::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses)
{
	TArray<TSharedRef<FInternetAddr>> BindArray = GetLocalBindAddresses();
	for (const auto& BindAddr : BindArray)
	{
		OutAddresses.Add(BindAddr);
	}

	return true;
}

TArray<TSharedRef<FInternetAddr>> FSocketSubsystemSteamCore::GetLocalBindAddresses()
{
	TArray<TSharedRef<FInternetAddr>> OutArray;

	FInternetAddrSteamCore* SteamAddr = nullptr;
	CSteamID SteamId;
	if (SteamUser())
	{
		SteamId = SteamUser()->GetSteamID();
		SteamAddr = new FInternetAddrSteamCore(*FUniqueNetIdSteam::Create(SteamId));
	}
	else if (SteamGameServer() && SteamGameServer()->BLoggedOn())
	{
		SteamId = SteamGameServer()->GetSteamID();
		SteamAddr = new FInternetAddrSteamCore(*FUniqueNetIdSteam::Create(SteamId));
	}
	else
	{
		SteamAddr = new FInternetAddrSteamCore();
	}

	OutArray.Add(MakeShareable(SteamAddr));
	return OutArray;
}

bool FSocketSubsystemSteamCore::AcceptP2PConnection(ISteamNetworking* SteamNetworkingPtr, const FUniqueNetIdSteam& RemoteId)
{
	if (SteamNetworkingPtr && RemoteId.IsValid() && !IsConnectionPendingRemoval(RemoteId, -1))
	{
		LogSteamCoreVerbose("Adding P2P connection information with user %s (Name: %s)", *RemoteId.ToString(), *RemoteId.ToDebugString());
		SteamNetworkingPtr->AcceptP2PSessionWithUser(RemoteId);
		UE_CLOG_ONLINE(m_AcceptedConnections.Contains(RemoteId.AsShared()), Warning, TEXT("User %s already exists in the connections list!!"), *RemoteId.ToString());
		m_AcceptedConnections.Add(RemoteId.AsShared(), FSteamP2PConnectionInfo(SteamNetworkingPtr));
		return true;
	}

	return false;
}

bool FSocketSubsystemSteamCore::P2PTouch(ISteamNetworking* SteamNetworkingPtr, const FUniqueNetIdSteam& SessionId, int32 ChannelId)
{
	LogSteamCoreVerbose("");
	if (!IsConnectionPendingRemoval(SessionId, ChannelId))
	{
		FSteamP2PConnectionInfo& ChannelUpdate = m_AcceptedConnections.FindOrAdd(SessionId.AsShared());
		ChannelUpdate.m_SteamNetworkingPtr = SteamNetworkingPtr;

		if (ChannelId != -1)
		{
			ChannelUpdate.AddOrUpdateChannel(ChannelId, FPlatformTime::Seconds());
		}
		return true;
	}

	return false;
}

void FSocketSubsystemSteamCore::P2PRemove(const FUniqueNetIdSteam& SessionId, int32 Channel)
{
	LogSteamCoreVerbose("");
	if (FSteamP2PConnectionInfo* ConnectionInfo = m_AcceptedConnections.Find(SessionId.AsShared()))
	{
		const bool bRemoveAllConnections = (Channel == -1);

		if (!IsConnectionPendingRemoval(SessionId, Channel))
		{
			if (bRemoveAllConnections)
			{
				LogSteamCoreVerbose("Replacing all existing removals with global removal for %s", *SessionId.ToString());

				for (TMap<FInternetAddrSteamCore, double>::TIterator It(m_DeadConnections); It; ++It)
				{
					if (*It->Key.m_SteamId == SessionId)
					{
						It.RemoveCurrent();
					}
				}
			}

			FInternetAddrSteamCore RemoveConnection(SessionId);
			RemoveConnection.SetPort(Channel);
			m_DeadConnections.Add(RemoveConnection, FPlatformTime::Seconds());

			LogSteamCoreVerbose("Removing P2P Session Id: %s, Channel: %d, IdleTime: %0.3f", *SessionId.ToDebugString(), Channel, ConnectionInfo ? (FPlatformTime::Seconds() - ConnectionInfo->m_LastReceivedTime) : 9999.f);
		}

		if (bRemoveAllConnections)
		{
			UE_CLOG_ONLINE((ConnectionInfo->m_ConnectedChannels.Num() > 0), Verbose, TEXT("Removing all channel connections for %s"), *SessionId.ToString());
			ConnectionInfo->m_ConnectedChannels.Empty();
		}
		else
		{
			const bool bWasRemoved = ConnectionInfo->m_ConnectedChannels.Remove(Channel) > 0;
			UE_CLOG_ONLINE(bWasRemoved, Verbose, TEXT("Removing channel %d from user %s"), Channel, *SessionId.ToString());
		}
	}
}

bool FSocketSubsystemSteamCore::IsConnectionPendingRemoval(const FUniqueNetIdSteam& SteamId, int32 Channel)
{
	FInternetAddrSteamCore RemovalToFind(SteamId);
	RemovalToFind.SetPort(-1);

	if (!m_DeadConnections.Contains(RemovalToFind))
	{
		if (Channel == -1)
		{
			return false;
		}

		RemovalToFind.SetPort(Channel);
		return m_DeadConnections.Contains(RemovalToFind);
	}

	return true;
}

bool FSocketSubsystemSteamCore::ShouldOverrideDefaultSubsystem() const
{
	bool bOverrideSetting;
	if (GConfig && GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bUseSteamNetworking"), bOverrideSetting, GEngineIni))
	{
		return bOverrideSetting;
	}
	return true;
}

bool FSocketSubsystemSteamCore::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SocketSubsystemSteamCore_Tick);

	const double CurSeconds = FPlatformTime::Seconds();

	bool bDumpSessionInfo = false;
	if ((CurSeconds - m_P2PDumpCounter) >= m_P2PDumpInterval)
	{
		m_P2PDumpCounter = CurSeconds;
		bDumpSessionInfo = true;
	}

	for (TUniqueNetIdMap<FSteamP2PConnectionInfo>::TConstIterator It(m_AcceptedConnections); It; ++It)
	{
		const FUniqueNetIdSteam& SessionId = FUniqueNetIdSteam::Cast(*It.Key());
		const FSteamP2PConnectionInfo& ConnectionInfo = It.Value();

		bool bExpiredSession = true;
		if (CurSeconds - ConnectionInfo.m_LastReceivedTime < m_P2PConnectionTimeout)
		{
			P2PSessionState_t SessionInfo;
			if (ConnectionInfo.m_SteamNetworkingPtr != nullptr && ConnectionInfo.m_SteamNetworkingPtr->GetP2PSessionState(SessionId, &SessionInfo))
			{
				bExpiredSession = false;

				if (bDumpSessionInfo)
				{
					LogSteamCoreVerbose("Dumping Steam P2P socket details:");
					LogSteamCoreVerbose("- Id: %s, Number of Channels: %d, IdleTime: %0.3f", *SessionId.ToDebugString(), ConnectionInfo.m_ConnectedChannels.Num(), (CurSeconds - ConnectionInfo.m_LastReceivedTime));

					DumpSteamP2PSessionInfo(SessionInfo);
				}
			}
			else if (ConnectionInfo.m_ConnectedChannels.Num() > 0)
			{
				LogSteamCoreVerbose("Failed to get Steam P2P session state for Id: %s, IdleTime: %0.3f", *SessionId.ToDebugString(), (CurSeconds - ConnectionInfo.m_LastReceivedTime));
			}
		}

		if (bExpiredSession)
		{
			P2PRemove(SessionId, -1);
		}
	}

	CleanupDeadConnections(false);

	return true;
}

bool FSocketSubsystemSteamCore::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
#if !UE_BUILD_SHIPPING
	if (FParse::Command(&Cmd, TEXT("dumpsteamsessions")))
	{
		DumpAllOpenSteamSessions();
		return true;
	}
#endif

	return false;
}

void FSocketSubsystemSteamCore::CleanupDeadConnections(bool bSkipLinger)
{
	LogSteamCoreVeryVerbose("");
	const double CurSeconds = FPlatformTime::Seconds();
	for (TMap<FInternetAddrSteamCore, double>::TIterator It(m_DeadConnections); It; ++It)
	{
		const FInternetAddrSteamCore& SteamConnection = It.Key();
		if (m_P2PCleanupTimeout == 0.0 || CurSeconds - It.Value() >= m_P2PCleanupTimeout || bSkipLinger)
		{
			if (const FSteamP2PConnectionInfo* ConnectionInfo = m_AcceptedConnections.Find(SteamConnection.m_SteamId))
			{
				bool bShouldRemoveUser = true;
				if (SteamConnection.GetPort() == -1)
				{
					LogSteamCoreVerbose("Closing all communications with user %s", *SteamConnection.ToString(false));
					ConnectionInfo->m_SteamNetworkingPtr->CloseP2PSessionWithUser(*SteamConnection.m_SteamId);
				}
				else
				{
					LogSteamCoreVerbose("Closing channel %d with user %s", SteamConnection.m_SteamChannel, *SteamConnection.ToString(false));
					ConnectionInfo->m_SteamNetworkingPtr->CloseP2PChannelWithUser(*SteamConnection.m_SteamId, SteamConnection.m_SteamChannel);
					if (ConnectionInfo->m_ConnectedChannels.Num() != 0)
					{
						bShouldRemoveUser = false;
						LogSteamCoreVerbose("%s still has %d open connections.", *SteamConnection.ToString(false), ConnectionInfo->m_ConnectedChannels.Num());
					}
					else
					{
						LogSteamCoreVerbose("%s has no more open connections! Going to remove", *SteamConnection.ToString(false));
					}
				}

				if (bShouldRemoveUser)
				{
					LogSteamCoreVerbose("%s has been removed.", *SteamConnection.ToString(false));
					m_AcceptedConnections.Remove(SteamConnection.m_SteamId);
				}
			}

			It.RemoveCurrent();
		}
	}
}

void FSocketSubsystemSteamCore::DumpSteamP2PSessionInfo(P2PSessionState_t& SessionInfo)
{
	LogSteamCoreVerbose("");
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem == nullptr)
	{
		return;
	}

	const TSharedRef<FInternetAddr> IpAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	IpAddr->SetIp(SessionInfo.m_nRemoteIP);
	IpAddr->SetPort(SessionInfo.m_nRemotePort);
	LogSteamCoreVerbose("- Detailed P2P session info:");
	LogSteamCoreVerbose("-- IPAddress: %s", *IpAddr->ToString(true));
	LogSteamCoreVerbose("-- ConnectionActive: %i, Connecting: %i, SessionError: %i, UsingRelay: %i", SessionInfo.m_bConnectionActive, SessionInfo.m_bConnecting, SessionInfo.m_eP2PSessionError, SessionInfo.m_bUsingRelay);
	LogSteamCoreVerbose("-- QueuedBytes: %i, QueuedPackets: %i", SessionInfo.m_nBytesQueuedForSend, SessionInfo.m_nPacketsQueuedForSend);
}

void FSocketSubsystemSteamCore::DumpAllOpenSteamSessions()
{
	LogSteamCoreVerbose("Current Connection Info: ");
	for (TUniqueNetIdMap<FSteamP2PConnectionInfo>::TConstIterator It(m_AcceptedConnections); It; ++It)
	{
		LogSteamCoreVerbose("- Connection %s", *It->Key->ToDebugString());
		//LogSteamCoreVerbose("--  Last Update Time: %d", It->Value.m_LastReceivedTime);
		FString ConnectedChannels(TEXT(""));
		for (int32 i = 0; i < It->Value.m_ConnectedChannels.Num(); ++i)
		{
			ConnectedChannels = FString::Printf(TEXT("%s %d"), *ConnectedChannels, It->Value.m_ConnectedChannels[i]);
		}
		LogSteamCoreVerbose("--  Channels:%s", *ConnectedChannels);
	}
}
#endif