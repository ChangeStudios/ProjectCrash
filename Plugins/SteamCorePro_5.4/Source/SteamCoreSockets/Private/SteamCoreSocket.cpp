/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocket.h"
#include "SteamCoreSocketsTypes.h"
#include "SteamCoreSocketsSubsystem.h"
#include "SteamCoreSocketsPrivate.h"

#if WITH_STEAMCORE

FSteamCoreSocket::FSteamCoreSocket(ESocketType InSocketType, const FString& InSocketDescription, const FName& InSocketProtocol)
	: FSocket(InSocketType, InSocketDescription, InSocketProtocol),
	  m_InternalHandle(k_HSteamNetConnection_Invalid),
	  m_SendMode(k_nSteamNetworkingSend_UnreliableNoNagle),
	  m_bShouldLingerOnClose(false),
	  m_bIsListenSocket(false),
	  m_bIsLANSocket(false),
	  m_bHasPendingData(false),
	  m_ClosureReason(k_ESteamNetConnectionEnd_App_Generic)
{
	m_SocketSubsystem = static_cast<FSteamCoreSocketsSubsystem*>(ISocketSubsystem::Get(STEAMCORE_SOCKETS_SUBSYSTEM));
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();
	m_PollGroup = k_HSteamNetPollGroup_Invalid;
}

FSteamCoreSocket::~FSteamCoreSocket()
{
	if (m_bHasPendingData && m_PendingData != nullptr)
	{
		m_PendingData->Release();
	}

	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();
	if (m_PollGroup != k_HSteamNetPollGroup_Invalid)
	{
		SocketInterface->DestroyPollGroup(m_PollGroup);
	}

	Close();
}

bool FSteamCoreSocket::Close()
{
	bool bWasSuccessful = false;

	STEAM_SDK_IGNORE_REDUNDANCY_START
	if (m_InternalHandle == k_HSteamListenSocket_Invalid || m_InternalHandle == k_HSteamNetConnection_Invalid)
	{
		UE_LOG(LogSockets, VeryVerbose, TEXT("SteamSockets: Socket is already cleaned up, ready for destruction."));
		return true;
	}
	STEAM_SDK_IGNORE_REDUNDANCY_END

	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();

	if (SocketInterface == nullptr)
	{
		UE_LOG(LogSockets, VeryVerbose, TEXT("SteamSockets: Socket interface not found, cannot close."));
		return false;
	}

	if (m_bIsListenSocket)
	{
		bWasSuccessful = SocketInterface->CloseListenSocket(m_InternalHandle);
		if (bWasSuccessful)
		{
			UE_LOG(LogSockets, Verbose, TEXT("SteamSockets: Closed listen socket %u via the API"), m_InternalHandle);
			m_SocketSubsystem->RemoveSocketsForListener(this);
			m_InternalHandle = k_HSteamListenSocket_Invalid;
		}
	}
	else
	{
		bWasSuccessful = SocketInterface->CloseConnection(m_InternalHandle, (int32)m_ClosureReason, "Connection Ended.", m_bShouldLingerOnClose);
		if (bWasSuccessful)
		{
			UE_LOG(LogSockets, Verbose, TEXT("SteamSockets: Closed socket %u via the API, reason %d"), m_InternalHandle, m_ClosureReason);
			m_SocketSubsystem->QueueRemoval(m_InternalHandle);
			m_InternalHandle = k_HSteamNetConnection_Invalid;
		}
	}

	return bWasSuccessful;
}

bool FSteamCoreSocket::Bind(const FInternetAddr& Addr)
{
	if (Addr.GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsIP ||
		Addr.GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		m_BindAddress = *((FInternetAddrSteamCoreSockets*)&Addr);
		return true;
	}

	return false;
}

bool FSteamCoreSocket::Connect(const FInternetAddr& Addr)
{
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();

	if (SocketInterface == nullptr)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamSockets: Socket Interface is null, cannot connect"));
		return false;
	}

	if (!Addr.IsValid())
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamSockets: Connection address is not valid. Cannot connect!"));
		m_SocketSubsystem->m_LastSocketError = SE_EINVAL;
		return false;
	}

	const FInternetAddrSteamCoreSockets SteamAddr = *((FInternetAddrSteamCoreSockets*)&Addr);
	if (GetProtocol() == FNetworkProtocolTypes::SteamCoreSocketsIP)
	{
		SteamNetworkingConfigValue_t LanOptions;
		LanOptions.m_eDataType = k_ESteamNetworkingConfig_Int32;
		LanOptions.m_eValue = k_ESteamNetworkingConfig_IP_AllowWithoutAuth;
		LanOptions.m_val.m_int32 = static_cast<int32>(m_bIsLANSocket);
		m_InternalHandle = SocketInterface->ConnectByIPAddress(SteamAddr, 1, &LanOptions);
	}
	else if (GetProtocol() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		m_InternalHandle = SocketInterface->ConnectP2P(SteamAddr, SteamAddr.GetPort(), 0, nullptr);
	}

	if (m_InternalHandle != k_HSteamNetConnection_Invalid)
	{
		UE_LOG(LogSockets, Verbose, TEXT("SteamSockets: Connection to %s initiated"), *Addr.ToString(false));
		m_SocketSubsystem->AddSocket(Addr, this);

		return true;
	}

	return false;
}

bool FSteamCoreSocket::Listen(int32 MaxBacklog)
{
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();

	if (SocketInterface == nullptr)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamSockets: Socket interface is null, cannot establish interface for listening!"));
		return false;
	}

	bool bWasSuccessful = false;
	m_bIsListenSocket = true;
	if (GetProtocol() == FNetworkProtocolTypes::SteamCoreSocketsIP)
	{
		SteamNetworkingConfigValue_t LanOptions;
		LanOptions.m_eDataType = k_ESteamNetworkingConfig_Int32;
		LanOptions.m_eValue = k_ESteamNetworkingConfig_IP_AllowWithoutAuth;
		LanOptions.m_val.m_int32 = static_cast<int32>(m_bIsLANSocket);
		m_InternalHandle = SocketInterface->CreateListenSocketIP(m_BindAddress, 1, &LanOptions);
	}
	else
	{
		m_InternalHandle = SocketInterface->CreateListenSocketP2P(m_BindAddress.GetPlatformPort(), 0, nullptr);
	}

	if (m_InternalHandle != k_HSteamListenSocket_Invalid)
	{
		bWasSuccessful = true;
		m_SocketSubsystem->AddSocket(m_BindAddress, this);

		m_PollGroup = SocketInterface->CreatePollGroup();
		SocketInterface->SetConnectionPollGroup(m_InternalHandle, m_PollGroup);
	}

	return bWasSuccessful;
}

class FSocket* FSteamCoreSocket::Accept(const FString& InSocketDescription)
{
	FSteamCoreSocket* NewSocket = static_cast<FSteamCoreSocket*>(m_SocketSubsystem->CreateSocket(FName("SteamClientSocket"), InSocketDescription, GetProtocol()));
	NewSocket->m_SendMode = m_SendMode;
	NewSocket->m_bShouldLingerOnClose = m_bShouldLingerOnClose;
	NewSocket->m_bIsLANSocket = m_bIsLANSocket;

	if (m_BindAddress.GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		NewSocket->m_BindAddress.SetPlatformPort(m_BindAddress.GetPlatformPort());
	}

	return NewSocket;
}

bool FSteamCoreSocket::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
	FInternetAddrSteamCoreSockets CurrentAddress;
	if (GetPeerAddress(CurrentAddress) && Destination == CurrentAddress)
	{
		return Send(Data, Count, BytesSent);
	}
	else if (m_SocketSubsystem)
	{
		const FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation* SocketInfo = m_SocketSubsystem->GetSocketInfo(Destination);
		if (SocketInfo != nullptr && SocketInfo->IsValid())
		{
			return SocketInfo->m_Socket->Send(Data, Count, BytesSent);
		}
	}

	return false;
}

bool FSteamCoreSocket::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
	BytesSent = 0;
	if (m_InternalHandle != k_HSteamNetConnection_Invalid && GetConnectionState() == SCS_Connected)
	{
		switch (FSteamCoreSocketsSubsystem::GetSteamSocketsInterface()->SendMessageToConnection(m_InternalHandle, (void*)Data, Count, m_SendMode, nullptr))
		{
		case k_EResultOK:
			m_SocketSubsystem->m_LastSocketError = SE_NO_ERROR;
			BytesSent = Count;
			return true;
			break;
		case k_EResultInvalidParam:
			m_SocketSubsystem->m_LastSocketError = SE_EINVAL;
			break;
		case k_EResultInvalidState:
			m_SocketSubsystem->m_LastSocketError = SE_EBADF;
			break;
		case k_EResultNoConnection:
			m_SocketSubsystem->m_LastSocketError = SE_ENOTCONN;
			break;
		case k_EResultIgnored:
			m_SocketSubsystem->m_LastSocketError = SE_SYSNOTREADY;
			break;
		case k_EResultLimitExceeded:
			m_SocketSubsystem->m_LastSocketError = SE_EPROCLIM;
			break;
		default:
			m_SocketSubsystem->m_LastSocketError = SE_EFAULT;
			break;
		}
		BytesSent = -1;
	}

	return false;
}

bool FSteamCoreSocket::Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags)
{
	BytesRead = -1;
	SteamNetworkingMessage_t* Message;
	int32 MessagesRead;

	if (RecvRaw(Message, 1, MessagesRead, Flags))
	{
		if (MessagesRead >= 1)
		{
			if (BufferSize >= 0 && Message->GetSize() <= (uint32)BufferSize)
			{
				FMemory::Memcpy(Data, Message->GetData(), BytesRead);
				Message->Release();
			}
			else
			{
				BytesRead = -1;
				m_SocketSubsystem->m_LastSocketError = SE_EMSGSIZE;
				return false;
			}
		}
		else
		{
			BytesRead = 0;
		}
		return true;
	}

	return false;
}

bool FSteamCoreSocket::RecvRaw(SteamNetworkingMessage_t*& Data, int32 MaxMessages, int32& MessagesRead, ESocketReceiveFlags::Type Flags)
{
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();
	if (SocketInterface == nullptr)
	{
		m_SocketSubsystem->m_LastSocketError = SE_SYSNOTREADY;
		return false;
	}

	if (m_InternalHandle == k_HSteamNetConnection_Invalid || (m_bIsListenSocket && m_InternalHandle == k_HSteamListenSocket_Invalid))
	{
		m_SocketSubsystem->m_LastSocketError = SE_EINVAL;
		return false;
	}

	const bool bIsPeeking = (Flags == ESocketReceiveFlags::Peek);
	if (bIsPeeking)
	{
		if (MaxMessages > 1)
		{
			UE_LOG(LogSockets, Warning, TEXT("SteamSockets: Recv cannot peek more than 1 message on the queue. Was asked for %d"), MaxMessages);
			MaxMessages = 1;
		}

		if (m_bHasPendingData)
		{
			MessagesRead = 1;
			return true;
		}
	}
	else if (m_bHasPendingData)
	{
		Data = m_PendingData;
		m_PendingData = nullptr;
		MessagesRead = 1;
		m_bHasPendingData = false;
		return true;
	}

	MessagesRead = (m_bIsListenSocket) ? SocketInterface->ReceiveMessagesOnPollGroup(m_PollGroup, ((bIsPeeking) ? &m_PendingData : &Data), MaxMessages) : SocketInterface->ReceiveMessagesOnConnection(m_InternalHandle, ((bIsPeeking) ? &m_PendingData : &Data), MaxMessages);

	if (MessagesRead >= 1)
	{
		if (bIsPeeking)
		{
			m_bHasPendingData = true;
		}
		m_SocketSubsystem->m_LastSocketError = SE_NO_ERROR;
		return true;
	}
	else if (MessagesRead == 0)
	{
		m_bHasPendingData = false;
		m_PendingData = nullptr;
		return true;
	}
	else
	{
		UE_LOG(LogSockets, Error, TEXT("SteamSockets: Recv Connection handle is marked as invalid! Is Listen Socket? %d"), m_bIsListenSocket);
		MessagesRead = -1;
	}

	m_SocketSubsystem->m_LastSocketError = SE_EFAULT;
	return false;
}

bool FSteamCoreSocket::HasPendingData(uint32& PendingDataSize)
{
	if (m_bHasPendingData)
	{
		if (m_PendingData != nullptr)
		{
			PendingDataSize = m_PendingData->GetSize();
			return true;
		}
		else
		{
			UE_LOG(LogSockets, Warning, TEXT("SteamSockets: HasPendingData flag is raised but the pendingdata object is null!"));
			m_bHasPendingData = false;
		}
	}

	int32 MessagesRead;
	PendingDataSize = 0;
	SteamNetworkingMessage_t* FakeMessage = nullptr;

	if (RecvRaw(FakeMessage, 1, MessagesRead, ESocketReceiveFlags::Peek))
	{
		if (MessagesRead >= 1 && m_PendingData != nullptr)
		{
			PendingDataSize = m_PendingData->GetSize();
			return true;
		}
	}

	return false;
}

ESocketConnectionState FSteamCoreSocket::GetConnectionState()
{
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();
	if (m_InternalHandle == k_HSteamNetConnection_Invalid || SocketInterface == nullptr)
	{
		return SCS_NotConnected;
	}

	SteamNetConnectionRealTimeStatus_t QuickSocketData;
	int nLanes = 0;
	SteamNetConnectionRealTimeLaneStatus_t* pLanes = NULL;
	if (SocketInterface->GetConnectionRealTimeStatus(m_InternalHandle, &QuickSocketData, nLanes, pLanes))
	{
		switch (QuickSocketData.m_eState)
		{
		case k_ESteamNetworkingConnectionState_Connected:
			return SCS_Connected;
		case k_ESteamNetworkingConnectionState_None:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			return SCS_ConnectionError;
		default:
			return SCS_NotConnected;
		}
	}

	return SCS_NotConnected;
}

void FSteamCoreSocket::SetSendMode(int32 NewSendMode)
{
	m_SendMode = NewSendMode;
}

bool FSteamCoreSocket::GetPeerAddress(FInternetAddr& OutAddr)
{
	ISteamNetworkingSockets* SocketInterface = FSteamCoreSocketsSubsystem::GetSteamSocketsInterface();
	FInternetAddrSteamCoreSockets& SteamAddr = (FInternetAddrSteamCoreSockets&)OutAddr;
	SteamNetConnectionInfo_t CurrentConnectionInfo;
	if (SocketInterface && SocketInterface->GetConnectionInfo(m_InternalHandle, &CurrentConnectionInfo))
	{
		if (CurrentConnectionInfo.m_identityRemote.IsInvalid())
		{
			if (CurrentConnectionInfo.m_addrRemote.IsIPv6AllZeros())
			{
				return false;
			}

			SteamAddr = CurrentConnectionInfo.m_addrRemote;
		}
		else
		{
			SteamAddr = CurrentConnectionInfo.m_identityRemote;
			SteamAddr.SetPlatformPort(m_BindAddress.GetPlatformPort());
		}
		return true;
	}

	return false;
}

bool FSteamCoreSocket::SetNoDelay(bool bIsNoDelay)
{
	STEAM_SDK_IGNORE_REDUNDANCY_START
	if (bIsNoDelay)
	{
		if (m_SendMode == k_nSteamNetworkingSend_Unreliable || m_SendMode == k_nSteamNetworkingSend_NoNagle || m_SendMode == k_nSteamNetworkingSend_UnreliableNoNagle)
		{
			m_SendMode = k_nSteamNetworkingSend_UnreliableNoDelay;
		}
		else if (m_SendMode == k_nSteamNetworkingSend_Reliable)
		{
			m_SendMode = k_nSteamNetworkingSend_ReliableNoNagle;
		}
	}
	else
	{
		if (m_SendMode == k_nSteamNetworkingSend_NoDelay)
		{
			m_SendMode = k_nSteamNetworkingSend_Unreliable;
		}
		else if (m_SendMode == k_nSteamNetworkingSend_UnreliableNoDelay)
		{
			m_SendMode = k_nSteamNetworkingSend_UnreliableNoNagle;
		}
		else if (m_SendMode == k_nSteamNetworkingSend_ReliableNoNagle)
		{
			m_SendMode = k_nSteamNetworkingSend_Reliable;
		}
	}
	STEAM_SDK_IGNORE_REDUNDANCY_END
	return true;
}

bool FSteamCoreSocket::SetLinger(bool bShouldLinger, int32 Timeout)
{
	m_bShouldLingerOnClose = bShouldLinger;
	return true;
}

void FSteamCoreSocket::SetLanOptions()
{
	if (SteamNetworkingUtils() && m_bIsLANSocket && m_InternalHandle != k_HSteamNetConnection_Invalid)
	{
		SteamNetworkingUtils()->SetConnectionConfigValueInt32(m_InternalHandle, k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
	}
}

bool FSteamCoreSocket::SetSendBufferSize(int32 Size, int32& NewSize)
{
	bool bSuccess = false;
	if (SteamNetworkingUtils())
	{
		if (SteamNetworkingUtils()->SetConnectionConfigValueInt32(m_InternalHandle, k_ESteamNetworkingConfig_SendBufferSize, Size))
		{
			bSuccess = true;
		}

		size_t ConfigValueSize = sizeof(int32);
		SteamNetworkingUtils()->GetConfigValue(k_ESteamNetworkingConfig_SendBufferSize, k_ESteamNetworkingConfig_Connection, m_InternalHandle, nullptr, &NewSize, &ConfigValueSize);
	}
	else
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamSockets: Setting buffer size requires access to network utilities!"));
		NewSize = -1;
	}

	return bSuccess;
}

bool FSteamCoreSocket::SetReceiveBufferSize(int32 Size, int32& NewSize)
{
	return SetSendBufferSize(Size, NewSize);
}

bool FSteamCoreSocket::Shutdown(ESocketShutdownMode Mode)
{
	return false;
}

bool FSteamCoreSocket::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
	return false;
}

bool FSteamCoreSocket::WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime)
{
	return false;
}

bool FSteamCoreSocket::SetReuseAddr(bool bAllowReuse)
{
	return true;
}

bool FSteamCoreSocket::SetRecvErr(bool bUseErrorQueue)
{
	return true;
}

bool FSteamCoreSocket::SetNonBlocking(bool bIsNonBlocking)
{
	return true;
}

bool FSteamCoreSocket::SetBroadcast(bool bAllowBroadcast)
{
	return false;
}

bool FSteamCoreSocket::JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	return false;
}

bool FSteamCoreSocket::JoinMulticastGroup(const FInternetAddr& GroupAddress)
{
	return false;
}

bool FSteamCoreSocket::LeaveMulticastGroup(const FInternetAddr& GroupAddress)
{
	return false;
}

bool FSteamCoreSocket::LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	return false;
}

bool FSteamCoreSocket::SetMulticastLoopback(bool bLoopback)
{
	return false;
}

bool FSteamCoreSocket::SetMulticastTtl(uint8 TimeToLive)
{
	return false;
}

bool FSteamCoreSocket::SetMulticastInterface(const FInternetAddr& InterfaceAddress)
{
	return false;
}
#endif