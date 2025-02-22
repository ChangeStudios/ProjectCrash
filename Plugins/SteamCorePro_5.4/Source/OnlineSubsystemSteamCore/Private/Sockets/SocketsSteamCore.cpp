/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Sockets/SocketsSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE
bool FSocketSteamCore::Shutdown(ESocketShutdownMode Mode)
{
	LogSteamCoreVerbose("");
	return false;
}

bool FSocketSteamCore::Close()
{
	LogSteamCoreVerbose("");
	return true;
}

bool FSocketSteamCore::Bind(const FInternetAddr& Addr)
{
	LogSteamCoreVerbose("");
	m_SteamChannel = Addr.GetPort();
	return true;
}

bool FSocketSteamCore::Connect(const FInternetAddr& Addr)
{
	return false;
}

bool FSocketSteamCore::Listen(int32 MaxBacklog)
{
	return false;
}

bool FSocketSteamCore::WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime)
{
	return false;
}

bool FSocketSteamCore::HasPendingData(uint32& PendingDataSize)
{
	if (m_SteamNetworkingPtr->IsP2PPacketAvailable(&PendingDataSize, m_SteamChannel))
	{
		return (PendingDataSize > 0);
	}

	return false;
}

FSocket* FSocketSteamCore::Accept(const FString& InSocketDescription)
{
	return nullptr;
}

FSocket* FSocketSteamCore::Accept(FInternetAddr& OutAddr, const FString& InSocketDescription)
{
	return nullptr;
}

bool FSocketSteamCore::SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination)
{
	bool bSuccess = false;
	if (m_SteamNetworkingPtr)
	{
		const FInternetAddrSteamCore& SteamDest = static_cast<const FInternetAddrSteamCore&>(Destination);
		if (*SteamDest.m_SteamId != *m_LocalSteamId)
		{
			if (m_SteamNetworkingPtr->SendP2PPacket(*SteamDest.m_SteamId, Data, Count, m_SteamSendMode, SteamDest.m_SteamChannel))
			{
				BytesSent = Count;
				bSuccess = true;
			}
		}
		else
		{
			UE_LOG(LogSockets, Warning, TEXT("Blocked FSocketSteamCoreworks::SendTo call, directed at localhost"));
		}
	}

	return bSuccess;
}

bool FSocketSteamCore::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
	BytesSent = 0;
	return false;
}

bool FSocketSteamCore::RecvFrom(uint8* Data, int32 BufferSize, int32& BytesRead, FInternetAddr& Source, ESocketReceiveFlags::Type Flags)
{
	if (Flags != ESocketReceiveFlags::None)
	{
		return false;
	}

	bool bSuccess = true;
	FInternetAddrSteamCore& SteamAddr = static_cast<FInternetAddrSteamCore&>(Source);

	uint32 MessageSize = 0;
	CSteamID SteamId;
	if (!m_SteamNetworkingPtr->ReadP2PPacket(Data, BufferSize, &MessageSize, &SteamId, m_SteamChannel))
	{
		MessageSize = 0;
		m_SocketSubsystem->m_LastSocketError = SE_EWOULDBLOCK;
		bSuccess = false;
	}
	else
	{
		SteamAddr.m_SteamId = FUniqueNetIdSteam::Create(SteamId);
		if (m_SocketSubsystem->P2PTouch(m_SteamNetworkingPtr, *SteamAddr.m_SteamId, m_SteamChannel))
		{
			m_SocketSubsystem->m_LastSocketError = SE_NO_ERROR;
		}
		else
		{
			MessageSize = 0;
			m_SocketSubsystem->m_LastSocketError = SE_UDP_ERR_PORT_UNREACH;
			bSuccess = false;
		}
	}

	SteamAddr.m_SteamChannel = m_SteamChannel;
	BytesRead = static_cast<int32>(MessageSize);

	if (BytesRead > BufferSize)
	{
		UE_LOG(LogSockets, Error, TEXT("FSocketSteamCore::RecvFrom: Failed to deserialize a packet (length of %d exceeds buffer length of %d), discarding!"), BytesRead, BufferSize);
		m_SocketSubsystem->m_LastSocketError = SE_EMSGSIZE;
		return false;
	}

	return bSuccess;
}

bool FSocketSteamCore::Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags)
{
	BytesRead = 0;
	return false;
}

bool FSocketSteamCore::Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime)
{
	return false;
}

ESocketConnectionState FSocketSteamCore::GetConnectionState()
{
	return SCS_NotConnected;
}

void FSocketSteamCore::GetAddress(FInternetAddr& OutAddr)
{
	FInternetAddrSteamCore& SteamAddr = static_cast<FInternetAddrSteamCore&>(OutAddr);
	SteamAddr.m_SteamId = m_LocalSteamId;
	SteamAddr.m_SteamChannel = m_SteamChannel;
}

bool FSocketSteamCore::GetPeerAddress(FInternetAddr& OutAddr)
{
	return false;
}

bool FSocketSteamCore::SetNonBlocking(bool bIsNonBlocking)
{
	return true;
}

bool FSocketSteamCore::SetBroadcast(bool bAllowBroadcast)
{
	return true;
}

bool FSocketSteamCore::JoinMulticastGroup(const FInternetAddr& GroupAddress)
{
	return false;
}

bool FSocketSteamCore::LeaveMulticastGroup(const FInternetAddr& GroupAddress)
{
	return false;
}

bool FSocketSteamCore::JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	return false;
}

bool FSocketSteamCore::LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress)
{
	return false;
}

bool FSocketSteamCore::SetMulticastLoopback(bool bLoopback)
{
	return false;
}

bool FSocketSteamCore::SetMulticastTtl(uint8 TimeToLive)
{
	return false;
}

bool FSocketSteamCore::SetMulticastInterface(const FInternetAddr& InterfaceAddress)
{
	return false;
}

bool FSocketSteamCore::SetReuseAddr(bool bAllowReuse)
{
	return true;
}

bool FSocketSteamCore::SetLinger(bool bShouldLinger, int32 Timeout)
{
	return true;
}

bool FSocketSteamCore::SetRecvErr(bool bUseErrorQueue)
{
	return true;
}

bool FSocketSteamCore::SetSendBufferSize(int32 Size, int32& NewSize)
{
	return true;
}

bool FSocketSteamCore::SetReceiveBufferSize(int32 Size, int32& NewSize)
{
	return true;
}

int32 FSocketSteamCore::GetPortNo()
{
	return m_SteamChannel;
}
#endif