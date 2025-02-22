/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocketsNetConnection.h"
#include "SteamCoreSocketsPrivate.h"
#include "IPAddressSteamCoreSockets.h"
#include "SteamCoreSocketsSubsystem.h"
#include "SteamCoreSocketsNetDriver.h"
#include "SteamCoreSocket.h"
#include "Net/DataChannel.h"
#include "CoreMinimal.h"
#include "PacketHandlers/StatelessConnectHandlerComponent.h"
#include "Templates/SharedPointer.h"

// Steam has the max value as 512 * 1024 (assumed bytes), which is unusable to us because UNetConnection asserts on any value greater than 1024
// Because of this, we cannot pass the value we get from Valve.
//
// As this value may change in a future Steam iteration without any notice, this is just set to 1024.
static const int32 MAX_PACKET = 1024;

void USteamCoreSocketsNetConnection::CleanUp()
{
#if WITH_STEAMCORE
	if (GetDriver() != nullptr && GetDriver()->GetSocketSubsystem() != nullptr && m_ConnectionSocket != nullptr)
	{
		FSteamCoreSocketsSubsystem* SocketSub = static_cast<FSteamCoreSocketsSubsystem*>(GetDriver()->GetSocketSubsystem());
		if (SocketSub != nullptr)
		{
			SocketSub->QueueRemoval(m_ConnectionSocket->m_InternalHandle);
		}
	}
#endif

	UNetConnection::CleanUp();
}

void USteamCoreSocketsNetConnection::InitBase(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
#if WITH_STEAMCORE
	// There is no packet overhead on this protocol, but since the UNetConnection will assert without a value greater than 0, we give it 1.
	UNetConnection::InitBase(InDriver, InSocket, InURL, InState, 
		((InMaxPacket == 0) ? MAX_PACKET : InMaxPacket),
		((InPacketOverhead == 0) ? 1 : InPacketOverhead));

	// Keep a reference to the socket.
	m_ConnectionSocket = static_cast<FSteamCoreSocket*>(InSocket);
#endif
}

void USteamCoreSocketsNetConnection::InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, EConnectionState InState,
	int32 InMaxPacket, int32 InPacketOverhead)
{
#if WITH_STEAMCORE
	// Call base so we set up all of our settings
	InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	// Grab the connection address from the URL
	RemoteAddr = InRemoteAddr.Clone();

	InitSendBuffer();
	SetClientLoginState(EClientLoginState::LoggingIn);
	SetExpectedClientLoginMsgType(NMT_Hello);
#endif
}

void USteamCoreSocketsNetConnection::InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
#if WITH_STEAMCORE
	// Call base so we set up all of our settings
	InitBase(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);

	if (InDriver == nullptr || InDriver->GetSocketSubsystem() == nullptr)
	{
		UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: InitLocalConnection: Unable to get SocketSubsystem"));
		Close();
		return;
	}

	// Grab the connection address from the URL
	RemoteAddr = InDriver->GetSocketSubsystem()->GetAddressFromString(InURL.Host);
	if (!RemoteAddr.IsValid() || !RemoteAddr->IsValid())
	{
		UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: Cannot create connection address from NetConnection URL"));
		Close();
		return;
	}
	RemoteAddr->SetPort(InURL.Port);

	InitSendBuffer();
#endif
}

void USteamCoreSocketsNetConnection::LowLevelSend(void* Data, int32 CountBits, FOutPacketTraits& Traits)
{
#if WITH_STEAMCORE
	// Grab the socket subsystem now, we'll use it later.
	FSteamCoreSocketsSubsystem* SocketSub = nullptr;
	if (GetDriver() != nullptr && GetDriver()->GetSocketSubsystem() != nullptr)
	{
		SocketSub = static_cast<FSteamCoreSocketsSubsystem*>(GetDriver()->GetSocketSubsystem());
	}

	if (m_ConnectionSocket != nullptr && SocketSub)
	{
		const uint8* SendData = reinterpret_cast<const uint8*>(Data);

		// Process any packet modifiers
		if (Handler.IsValid() && !Handler->GetRawSend())
		{
			const ProcessedPacket ProcessedData = Handler->Outgoing(reinterpret_cast<uint8*>(Data), CountBits, Traits);

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

		int32 BytesSent = 0;		
		int32 BytesToSend = FMath::DivideAndRoundUp(CountBits, 8);

		// Make sure that we're within the acceptable size range.
		if (BytesToSend > k_cbMaxSteamNetworkingSocketsMessageSizeSend)
		{
			UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: LowLevelSend: Attempted to send %d bytes of data, which is over the %d maximum"), 
				BytesToSend, k_cbMaxSteamNetworkingSocketsMessageSizeSend);
			SocketSub->m_LastSocketError = SE_EMSGSIZE;
			return;
		}
		
		if (BytesToSend > 0)
		{
			if (!m_ConnectionSocket->Send(SendData, BytesToSend, BytesSent))
			{
				UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: LowLevelSend: Could not send %d bytes of data got error %d"), BytesToSend, (int32)SocketSub->GetLastErrorCode());
			}
		}
	}
	else if (SocketSub)
	{
		UE_LOG(LogNet, Error, TEXT("SteamCoreSockets: LowLevelSend: Attempted to send data over a netconnection that has no socket!"));
		SocketSub->m_LastSocketError = SE_EPROCLIM;
	}
#endif
}

FString USteamCoreSocketsNetConnection::LowLevelGetRemoteAddress(bool bAppendPort)
{
#if WITH_STEAMCORE
	// If we have a remote addr set already (should always have this set)
	if (RemoteAddr.IsValid() && RemoteAddr->IsValid())
	{
		return RemoteAddr->ToString(bAppendPort);
	}

	// Otherwise, attempt to pull it from the socket itself.
	FInternetAddrSteamCoreSockets AddressData;
	if (m_ConnectionSocket != nullptr)
	{
		m_ConnectionSocket->GetPeerAddress(AddressData);
		if (AddressData.IsValid())
		{
			return AddressData.ToString(bAppendPort);
		}
	}
#endif
	return TEXT("Invalid");
}

FString USteamCoreSocketsNetConnection::LowLevelDescribe()
{
	return FString::Printf(TEXT("Addr=%s"), *RemoteAddr->ToString(true));
}

void USteamCoreSocketsNetConnection::HandleRecvMessage(void* InData, int32 SizeOfData, const FInternetAddr* InFormattedAddress)
{
#if WITH_STEAMCORE
	uint8* RecvData = reinterpret_cast<uint8*>(InData);
	if (RecvData == nullptr || InFormattedAddress == nullptr)
	{
		return;
	}

	if (m_bInConnectionlessHandshake)
	{
		USteamCoreSocketsNetDriver* SteamNetDriver = static_cast<USteamCoreSocketsNetDriver*>(Driver);
		if (SteamNetDriver && !SteamNetDriver->ArePacketHandlersDisabled() && SteamNetDriver->ConnectionlessHandler.IsValid() && SteamNetDriver->StatelessConnectComponent.IsValid())
		{
			bool bRestartedHandshake = false;
			bool bPassedChallenge = false;

			TSharedPtr<const FInternetAddr> SharedAddress = InFormattedAddress->Clone();
			const ProcessedPacket RawPacket = SteamNetDriver->ConnectionlessHandler->IncomingConnectionless(SharedAddress, RecvData, SizeOfData);
			TSharedPtr<StatelessConnectHandlerComponent> StatelessConnect = SteamNetDriver->StatelessConnectComponent.Pin();

			if (!RawPacket.bError && StatelessConnect->HasPassedChallenge(SharedAddress, bRestartedHandshake) && !bRestartedHandshake)
			{
				// Set the initial packet sequence from the handshake data
				if (StatelessConnect.IsValid())
				{
					int32 ServerSequence = 0;
					int32 ClientSequence = 0;

					StatelessConnect->GetChallengeSequence(ServerSequence, ClientSequence);

					InitSequence(ClientSequence, ServerSequence);
				}

				// Start handshaking now
				if (Handler.IsValid())
				{
					Handler->BeginHandshaking();
				}

				UE_LOG(LogNet, Log, TEXT("SteamCoreSockets: Connectionless handshake complete"));
				m_bInConnectionlessHandshake = false;
				
				// Reset the challenge data for the future
				if (StatelessConnect.IsValid())
				{
					StatelessConnect->ResetChallengeData();
				}
				
				SizeOfData = FMath::DivideAndRoundUp(RawPacket.CountBits, 8);
				if (SizeOfData > 0)
				{
					RecvData = RawPacket.Data;
				}
				else
				{
					return;
				}
			}
		}
	}
	UNetConnection::ReceivedRawPacket(RecvData, SizeOfData);
#endif
}
