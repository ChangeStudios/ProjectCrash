/**
* Copyright (C) 2017-2024 eelDev AB
*
*/


#include "SteamCoreSocketsNetDriver.h"
#include "SteamCoreSocketsPrivate.h"
#include "SocketSubsystem.h"
#include "SteamCoreSocketsTypes.h"
#include "SteamCoreSocketsModule.h"
#include "SteamCoreSocket.h"
#include "SteamCoreSocketsNetConnection.h"
#include "SteamCoreSocketsSubsystem.h"
#include "IPAddressSteamCoreSockets.h"
#include "Engine/NetworkDelegates.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "PacketHandler.h"

void USteamCoreSocketsNetDriver::PostInitProperties()
{
	Super::PostInitProperties();
}

void USteamCoreSocketsNetDriver::Shutdown()
{
#if WITH_STEAMCORE
	UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Shutdown called on netdriver"));
	FSteamCoreSocketsSubsystem* SocketSub = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());
	if (SocketSub && m_Socket)
	{
		FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation* SocketInfo = SocketSub->GetSocketInfo(m_Socket->m_InternalHandle);
		// Make sure to remove any netdriver information as we are already shutting down.
		if (SocketInfo)
		{
			SocketInfo->m_NetDriver = nullptr;
			SocketInfo->MarkForDeletion();
		}

		m_Socket = nullptr;
	}
#endif

	Super::Shutdown();
}

bool USteamCoreSocketsNetDriver::IsAvailable() const
{
#if WITH_STEAMCORE
	const FSteamCoreSocketsSubsystem* SocketSub = static_cast<const FSteamCoreSocketsSubsystem*>(ISocketSubsystem::Get(STEAMCORE_SOCKETS_SUBSYSTEM));
	// Check if we have client handles and we active.
	if (SocketSub && SocketSub->IsSteamInitialized() && FSteamCoreSocketsModule::Get().IsSteamCoreSocketsEnabled())
	{
		return true;
	}
#endif
	
	return false;
}

bool USteamCoreSocketsNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
#if WITH_STEAMCORE
	// Initialize base class
	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Failed to initialize the base UNetDriver"));
		return false;
	}

	if (!IsAvailable())
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: SteamCoreSockets are not enabled, cannot use netdriver!"));
		return false;
	}

	FName SocketAddressType = NAME_None;
	const bool bIsLAN = URL.HasOption(TEXT("bIsLanMatch"));
	const bool bForcedIP = (bIsLAN || URL.HasOption(TEXT("bPassthrough")));
	FSteamCoreSocketsSubsystem* SocketSub = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());
	if (!SocketSub)
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Unable to get SocketSubsystem"));
		return false;
	}

	// Check to see if we're going to be using a P2P address
	const bool bIsUsingSteamAddrs = (SocketSub->IsUsingRelayNetwork());

	// This case is very special to the Steam framework and should not be followed as a template for other protocols.
	// On Steam Sockets we need to be logged in as a dedicated server if we are hosted or just using the relay network.
	// Because of this, we need to delay our actions and inquire for a login with the Steam network.
	m_bIsDelayedNetworkAccess = !bInitAsClient &&
		bIsUsingSteamAddrs &&
		!SocketSub->IsLoggedInToSteam() && !bForcedIP;

	// Delayed access and clients that will be hosting using SteamIDs will skip over the address determination and just use the P2P network.
	if (m_bIsDelayedNetworkAccess || (!bInitAsClient && !IsRunningDedicatedServer() && bIsUsingSteamAddrs && !bForcedIP))
	{
		SocketAddressType = FNetworkProtocolTypes::SteamCoreSocketsP2P;
	}
	else if (bForcedIP) // Support LAN Matches over Steam Sockets
	{
		SocketAddressType = FNetworkProtocolTypes::SteamCoreSocketsIP;
	}
	else
	{
		// This is the typical behavior of a connection in a NetDriver.
		// Figure out what kind of socket we'll need by parsing the Host field.
		TSharedPtr<FInternetAddr> SocketAddressHost = SocketSub->GetAddressFromString(URL.Host);
		if (SocketAddressHost.IsValid() && SocketAddressHost->IsValid())
		{
			SocketAddressType = SocketAddressHost->GetProtocolType();
		}
	}

	// Create the socket itself
	FString SocketDescription = (bInitAsClient) ? TEXT("Unreal client (Steam Sockets)") : TEXT("Unreal server (Steam Sockets)");
	m_Socket = static_cast<FSteamCoreSocket*>(SocketSub->CreateSocket(FName(TEXT("SteamSocket")), SocketDescription, SocketAddressType));
	m_Socket->SetNoDelay(true);
	
	// Set up LAN flags early so that we make sure that the handshake doesn't go south when we don't have
	// a stable connection to the Steam backend.
	if (bIsLAN)
	{
		m_Socket->m_bIsLANSocket = true;
	}

	// On Servers, there's a possibility that the SocketAddressType is None due to not having enough information to determine
	// the user's intent. Because of this, the SocketSubsystem will have determined a correct protocol for us to use. We need
	// to update our address type so that we can find a good binding address to use later.
	if (SocketAddressType.IsNone())
	{
		SocketAddressType = m_Socket->GetProtocol();
	}

	// Delayed server login causes us to have a valid bind address much later than an usual process.
	// Because of this, a normal flow should be skipped over and use a forced P2P address.
	if (!m_bIsDelayedNetworkAccess)
	{
		UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: Looking for a binding address that matches protocol %s"), *SocketAddressType.ToString());

		// Set the binding address.
		TArray<TSharedRef<FInternetAddr>> BindAddresses = SocketSub->GetLocalBindAddresses();
		if (BindAddresses.Num() > 0)
		{
			// Attempt to find a binding address that matches protocol of the connection destination
			for (int32 i = 0; i < BindAddresses.Num(); ++i)
			{
				UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: Looking at binding address %s"), *BindAddresses[i]->ToString(true));
				if (BindAddresses[i]->GetProtocolType() == SocketAddressType)
				{
					UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: Picked this binding address."));
					LocalAddr = BindAddresses[i]->Clone();
					break;
				}
			}
		}

		// Check for binding address issues.
		if (!LocalAddr.IsValid())
		{
			UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: Could not determine the binding address!"));
			return false;
		}
	}
	else
	{
		// Create a blank address, this will get filled in later when we log in.
		LocalAddr = SocketSub->CreateInternetAddr();
	}

	// Set the port
	LocalAddr->SetPort(URL.Port);

	// This should never fail.
	if (m_Socket->Bind(*LocalAddr) == false)
	{
		// If this fails, we somehow created a binding address that we cannot use.
		UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: Invalid binding address used to create socket"));
		return false;
	}
	
	return true;
#else
	return false;
#endif
}

bool USteamCoreSocketsNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
#if WITH_STEAMCORE
	if (!InitBase(true, InNotify, ConnectURL, false, Error))
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: InitConnect failed while setting up base information"));
		return false;
	}

	// Create the NetConnection and channels
	ServerConnection = NewObject<USteamCoreSocketsNetConnection>(NetConnectionClass);
	ServerConnection->InitLocalConnection(this, m_Socket, ConnectURL, USOCK_Pending);
	CreateInitialClientChannels();

	// Set the sequence flags if we have no packet handlers
	if (ArePacketHandlersDisabled())
	{
		ServerConnection->InitSequence(4, 4);
	}

	// Grab the subsystem
	FSteamCoreSocketsSubsystem* SteamSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());
	check(SteamSubsystem);

	// Figure out where we are going.
	TSharedPtr<FInternetAddr> SocketAddressHost = SteamSubsystem->GetAddressFromString(ConnectURL.Host);
	if (!SocketAddressHost.IsValid() || !SocketAddressHost->IsValid())
	{
		UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: Could not obtain the address to connect to, had input %s"), *ConnectURL.Host);
		return false;
	}

	SocketAddressHost->SetPort(ConnectURL.Port);

	// Attempt to connect to the server
	if (!m_Socket->Connect(*SocketAddressHost))
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Could not connect to address %s, got error code %d"), *SocketAddressHost->ToString(true), 
			(int32)SteamSubsystem->GetLastErrorCode());
		return false;
	}

	// Link connection information for later
	SteamSubsystem->LinkNetDriver(m_Socket, this);

	UE_LOG(LogNet, Log, TEXT("Game client on port %i, rate %i"), ConnectURL.Port, ServerConnection->CurrentNetSpeed);

	return true;
#else
	return false;
#endif

}

bool USteamCoreSocketsNetDriver::InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error)
{
#if WITH_STEAMCORE
	if (!InitBase(false, InNotify, LocalURL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: InitListen failed while setting up base information"));
		return false;
	}

	InitConnectionlessHandler();


	// Tell the SteamAPI to actually start listening.
	// This will actually create the socket with the SDK
	if (!m_bIsDelayedNetworkAccess && m_Socket->Listen(0) == false)
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: InitListen failed to start listening on the Steam Network."));
		m_Socket->SetClosureReason(k_ESteamNetConnectionEnd_Misc_SteamConnectivity);

		return false;
	}

	// Watch for connections that come in.
	FSteamCoreSocketsSubsystem* SteamSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());
	check(SteamSubsystem);

	if (!m_bIsDelayedNetworkAccess)
	{
		SteamSubsystem->LinkNetDriver(m_Socket, this);
	}
	else
	{
		SteamSubsystem->AddDelayedListener(m_Socket, this);
	}

	UE_LOG(LogNet, Log, TEXT("%s started listening on %d"), *GetDescription(), LocalURL.Port);
	UE_CLOG((!m_bIsDelayedNetworkAccess && !UE_BUILD_SHIPPING), LogNet, Log, TEXT("SteamCoreSockets: Listening with handle %u"), m_Socket->m_InternalHandle);
	return true;
#else
	return false;
#endif
}

void USteamCoreSocketsNetDriver::TickDispatch(float DeltaTime)
{
#if WITH_STEAMCORE
	// Do this first to clean up dead connections so we don't waste time processing them.
	UNetDriver::TickDispatch(DeltaTime);

	bool bIsAServer = (ServerConnection == nullptr);
	FSteamCoreSocketsSubsystem* SteamSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());

	// Receive packets until we cannot any longer.
	while (!m_bIsDelayedNetworkAccess)
	{
		// This could be placed out of the loop, however if we thread API event calls, this will need to be checked on every recv
		if (m_Socket != nullptr)
		{

#if !UE_BUILD_SHIPPING
			// This code causes peek cycles to run in order to test if the functionality works properly.
			if (SteamSubsystem && SteamSubsystem->m_bShouldTestPeek)
			{
				uint32 PendingDataSize;
				if (m_Socket->HasPendingData(PendingDataSize))
				{
					UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: handle %u has %u data pending on the socket"), m_Socket->m_InternalHandle, PendingDataSize);
				}
			}
#endif
			int32 BytesRead = 0;
			SteamNetworkingMessage_t* Message;
			// Attempt to grab a message using the Socket
			if (m_Socket->RecvRaw(Message, 1, BytesRead) && BytesRead > 0 && Message != nullptr)
			{
#if UE_VERSION_OLDER_THAN(5,0,0)
				USteamCoreSocketsNetConnection* ConnectionToHandleMessage = static_cast<USteamCoreSocketsNetConnection*>((bIsAServer) ? FindClientConnectionForHandle(Message->m_conn) : ServerConnection);
#else
				USteamCoreSocketsNetConnection* ConnectionToHandleMessage = static_cast<USteamCoreSocketsNetConnection*>((bIsAServer) ? FindClientConnectionForHandle(Message->m_conn) : ToRawPtr(ServerConnection));
#endif

				// Grab sender information for the purposes of logging
				FInternetAddrSteamCoreSockets MessageSender(Message->m_identityPeer);

				// Set the P2P channel information if we're not over IP (which will already have the right data set)
				if (MessageSender.GetProtocolType() != FNetworkProtocolTypes::SteamCoreSocketsIP)
				{
					MessageSender.SetPort(Message->m_nChannel);
				}

				// Process the message for this connection.
				if (ConnectionToHandleMessage != nullptr)
				{
					UE_LOG(LogNet, VeryVerbose, TEXT("SteamCoreSockets: Recieved packet from %s with size %d"), *MessageSender.ToString(true), Message->GetSize());
					ConnectionToHandleMessage->HandleRecvMessage(Message->m_pData, Message->GetSize(), &MessageSender);
					Message->Release();
					continue;
				}
				else
				{
					UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Could not find connection information for sender %s (handle: %u)"), *MessageSender.ToString(true), Message->m_conn);
					Message->Release();
					continue;
				}
			}
			else if (BytesRead == 0)
			{
				// We have no more messages, leave.
				UE_LOG(LogNet, VeryVerbose, TEXT("SteamCoreSockets: Exhausted message, exiting loop."));
				break;
			}
			else
			{
				// In theory, we should have no information what so ever about connections that failed.
				// The sender should have no information in here and this should only happen if our handle is null.

				// A later disconnection event message from the API should handle cleanup of this object.
				UE_CLOG(SteamSubsystem, LogNet, Warning, TEXT("SteamCoreSockets: Could not recv message, got error code %d"), SteamSubsystem->GetLastErrorCode());
				break;
			}
		}
		else
		{
			// Silently exit if the socket is invalid.
			return;
		}
	}
#endif
	return;
}

void USteamCoreSocketsNetDriver::LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
#if WITH_STEAMCORE
	if (Address.IsValid() && Address->IsValid() && m_Socket != nullptr)
	{
		const uint8* SendData = reinterpret_cast<const uint8*>(Data);
		if (ConnectionlessHandler.IsValid())
		{
			const ProcessedPacket ProcessedData =
				ConnectionlessHandler->OutgoingConnectionless(Address, (uint8*)SendData, CountBits, Traits);

			if (!ProcessedData.bError)
			{
				SendData = ProcessedData.Data;
				CountBits = ProcessedData.CountBits;
			}
			else
			{
				CountBits = 0;
			}
		}

		if (CountBits > 0)
		{
			int32 BytesToSend = FMath::DivideAndRoundUp(CountBits, 8);
			int32 SentBytes = 0;

			// Our sendto will find the correct socket to send over.
			if (!m_Socket->SendTo(SendData, BytesToSend, SentBytes, *Address))
			{
				UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: LowLevelSend: Could not send %d data over socket to %s!"), BytesToSend, *Address->ToString(true));
			}
		}
	}
	else
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: LowLevelSend: Address or Socket was invalid, could not send data."));
	}
#else
	Super::LowLevelSend(Address, Data, CountBits, Traits);
#endif
}

void USteamCoreSocketsNetDriver::LowLevelDestroy()
{
	UNetDriver::LowLevelDestroy();

#if WITH_STEAMCORE
	FSteamCoreSocketsSubsystem* SteamSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());
	if (m_Socket != nullptr && !HasAnyFlags(RF_ClassDefaultObject))
	{
		SteamSubsystem->QueueRemoval(m_Socket->m_InternalHandle);
		m_Socket = nullptr;

		UE_LOG(LogNet, Log, TEXT("%s shut down"), *GetDescription());
	}
#endif
}

class ISocketSubsystem* USteamCoreSocketsNetDriver::GetSocketSubsystem()
{
	return ISocketSubsystem::Get(STEAMCORE_SOCKETS_SUBSYSTEM);
}

bool USteamCoreSocketsNetDriver::IsNetResourceValid(void)
{
#if UE_VERSION_OLDER_THAN(5,0,0)
	return m_Socket != nullptr && (!ServerConnection || (ServerConnection && ServerConnection->State == USOCK_Open));
#else
	return m_Socket != nullptr && (!ServerConnection || (ServerConnection && ServerConnection->GetConnectionState() == USOCK_Open));
#endif
}

bool USteamCoreSocketsNetDriver::ArePacketHandlersDisabled() const
{
#if !UE_BUILD_SHIPPING
	return FParse::Param(FCommandLine::Get(), TEXT("NoPacketHandler"));
#else
	return false;
#endif
}

void USteamCoreSocketsNetDriver::ResetSocketInfo(const FSteamCoreSocket* RemovedSocket)
{
#if WITH_STEAMCORE
	const SteamCoreSocketHandles SocketHandle = RemovedSocket->m_InternalHandle;
#if UE_VERSION_OLDER_THAN(5,0,0)
	USteamCoreSocketsNetConnection* SocketConnection = static_cast<USteamCoreSocketsNetConnection*>(ServerConnection ? ServerConnection : FindClientConnectionForHandle(SocketHandle));
#else
	USteamCoreSocketsNetConnection* SocketConnection = static_cast<USteamCoreSocketsNetConnection*>(ServerConnection ? ToRawPtr(ServerConnection) : FindClientConnectionForHandle(SocketHandle));
#endif

	if (SocketConnection)
	{
		SocketConnection->ClearSocket();
	}

	// If this netdriver has the same socket pointer, go ahead and remove it.
	if (m_Socket == RemovedSocket)
	{
		m_Socket = nullptr;
	}
#endif
}

UNetConnection* USteamCoreSocketsNetDriver::FindClientConnectionForHandle(SteamCoreSocketHandles SocketHandle)
{
#if WITH_STEAMCORE
	for (const auto& ClientConnection : ClientConnections)
	{
		const USteamCoreSocketsNetConnection* SteamConnection = static_cast<USteamCoreSocketsNetConnection*>(ClientConnection);
		if (SteamConnection && SteamConnection->GetRawSocket() != nullptr)
		{
			const FSteamCoreSocket* SteamSocket = SteamConnection->GetRawSocket();
			if (SteamSocket && SteamSocket->m_InternalHandle == SocketHandle)
			{
				return ClientConnection;
			}
		}
	}
#endif
	return nullptr;
}

void USteamCoreSocketsNetDriver::OnConnectionCreated(SteamCoreSocketHandles ListenParentHandle, SteamCoreSocketHandles SocketHandle)
{
#if WITH_STEAMCORE
	FSteamCoreSocketsSubsystem* SocketSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(GetSocketSubsystem());

	// If this netdriver does not hold the listener for the incoming connection, do not handle it
	// This makes sure cleanup of listener socket later does not destroy sockets it does not own.
	// (useful for beacons)
	if (m_Socket == nullptr || ListenParentHandle != ((FSteamCoreSocket*)m_Socket)->m_InternalHandle || !SocketSubsystem)
	{
		return;
	}

	// Absolutely make sure that we can interface with the SteamAPI
	check(FSteamCoreSocketsSubsystem::GetSteamSocketsInterface());

	// Unlike the other SteamNetworking functionality, connections that we don't want cannot just be ignored
	// So instead of processing everything and then disconnecting unwanted connections, we drop them immediately.
	if (Notify != nullptr && Notify->NotifyAcceptingConnection() == EAcceptConnection::Accept)
	{
		// Accept the connection with the API. We'll want to do this as quick as possible.
		EResult AcceptedResult = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface()->AcceptConnection(SocketHandle);
		if (AcceptedResult != k_EResultOK)
		{
			// We can fail here because the client aborted or something happened to our connection to the network
			UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: Cannot accept connection due to error %d"), (int32)AcceptedResult);
			SocketSubsystem->m_LastSocketError = SE_ECONNRESET;
			return;
		}

		// Create the new connection
		USteamCoreSocketsNetConnection* NewConnection = NewObject<USteamCoreSocketsNetConnection>(NetConnectionClass);
		check(NewConnection);

		// Create the bookkeeping and set up the socket.
		FInternetAddrSteamCoreSockets ConnectedAddr;
		FSteamCoreSocket* NewSocket = static_cast<FSteamCoreSocket*>(m_Socket->Accept(TEXT("AcceptedSocket")));
		NewSocket->m_InternalHandle = SocketHandle;

		SocketSubsystem->GetSteamSocketsInterface()->SetConnectionPollGroup(SocketHandle, m_Socket->m_PollGroup);

		NewSocket->GetPeerAddress(ConnectedAddr);
		SocketSubsystem->AddSocket(ConnectedAddr, NewSocket, m_Socket);
		SocketSubsystem->LinkNetDriver(NewSocket, this);

		EConnectionState RemoteConnectionState = USOCK_Pending;

		SteamNetConnectionRealTimeStatus_t ConnStatus;
		// We might already have a full route to the user already, which will not signal an event later (for the listener) that would otherwise update the connection state
		// Because of this, we need to quickly query the new connection status to determine where we are in the connection flow
		// This is very unlikely unless we already had a previous route to this connection.

		int nLanes = 0;
		SteamNetConnectionRealTimeLaneStatus_t* pLanes = NULL;
		
		if (FSteamCoreSocketsSubsystem::GetSteamSocketsInterface()->GetConnectionRealTimeStatus(SocketHandle, &ConnStatus, nLanes, pLanes) && ConnStatus.m_eState == k_ESteamNetworkingConnectionState_Connected)
		{
			// We have already found a route and made a connection, skip out of pending.
			RemoteConnectionState = USOCK_Open;
		}
		
		// Create the net connection structures and add the connections.
		NewConnection->InitRemoteConnection(this, NewSocket, World ? World->URL : FURL(), ConnectedAddr, RemoteConnectionState);

		if (ArePacketHandlersDisabled())
		{
			// Set up the sequence numbers. Valve uses their own, but so do we so to prevent
			// reinventing the wheel, we'll set up our sequence numbers to be some random garbage
			NewConnection->InitSequence(4, 4);

			// Attempt to start the PacketHandler handshakes (we do not support stateless connect)
			if (NewConnection->Handler.IsValid())
			{
				NewConnection->Handler->BeginHandshaking();
			}
		}
		else if (ConnectionlessHandler.IsValid() && StatelessConnectComponent.IsValid())
		{
			NewConnection->FlagForHandshake();
		}

		Notify->NotifyAcceptedConnection(NewConnection);
		AddClientConnection(NewConnection);

		UE_LOG(LogNet, Log, TEXT("SteamCoreSockets: New connection (%u) over listening socket accepted %s"), SocketHandle, *ConnectedAddr.ToString(true));
	}
	else
	{
		FSteamCoreSocketsSubsystem::GetSteamSocketsInterface()->CloseConnection(SocketHandle, k_ESteamNetConnectionEnd_App_Generic, "Connection rejected", false);
		UE_LOG(LogNet, Log, TEXT("SteamCoreSockets: New connection over listening socket rejected."));
	}
#endif
}

void USteamCoreSocketsNetDriver::OnConnectionUpdated(SteamCoreSocketHandles SocketHandle, int32 NewState)
{
#if WITH_STEAMCORE
	// While this function can also get the relay status flags, we really only
	// care about the connected state flag.
	if (NewState == k_ESteamNetworkingConnectionState_Connected)
	{
#if UE_VERSION_OLDER_THAN(5,0,0)
		UNetConnection* SocketConnection = (ServerConnection) ? ServerConnection : FindClientConnectionForHandle(SocketHandle);
#else
		UNetConnection* SocketConnection = (ServerConnection) ? ToRawPtr(ServerConnection) : FindClientConnectionForHandle(SocketHandle);
#endif
		if (SocketConnection)
		{
#if UE_VERSION_OLDER_THAN(5,0,0)
			SocketConnection->State = USOCK_Open;
#else
			SocketConnection->SetConnectionState(USOCK_Open);
#endif
		}

		UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: Connection established with user with socket id: %u"), SocketHandle);

		// For clients, we need to match the settings of the server
		if (ServerConnection && ArePacketHandlersDisabled())
		{
			// Attempt to start the PacketHandler handshakes (we do not support stateless connect)
			// The PendingNetGame will also do it but we can't actually send the packets for it until we're connected.
			if (ServerConnection->Handler.IsValid())
			{
				ServerConnection->Handler->BeginHandshaking();
			}
		}
	}
#endif
}

void USteamCoreSocketsNetDriver::OnConnectionDisconnected(SteamCoreSocketHandles SocketHandle)
{
#if WITH_STEAMCORE
#if UE_VERSION_OLDER_THAN(5,0,0)
	UNetConnection* SocketConnection = ServerConnection ? ServerConnection : FindClientConnectionForHandle(SocketHandle);
#else
	UNetConnection* SocketConnection = ServerConnection ? ToRawPtr(ServerConnection) : FindClientConnectionForHandle(SocketHandle);
#endif
	if (SocketConnection)
	{
#if UE_VERSION_OLDER_THAN(5,0,0)
		SocketConnection->State = USOCK_Closed;
#else
		SocketConnection->SetConnectionState(USOCK_Closed);
#endif
	}

	UE_LOG(LogNet, Verbose, TEXT("SteamCoreSockets: Connection dropped with user with socket id: %u"), SocketHandle);
#endif
}