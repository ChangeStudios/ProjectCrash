/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "SteamCoreSocketsPackage.h"
#include "SteamCoreSocketsPrivate.h"
#include "IPAddressSteamCoreSockets.h"
#include "Sockets.h"
#include "SocketTypes.h"

#if WITH_STEAMCORE
class FSteamCoreSocket : public FSocket
{
PACKAGE_SCOPE:
	HSteamNetPollGroup m_PollGroup;
	HSteamNetConnection m_InternalHandle;
	FInternetAddrSteamCoreSockets m_BindAddress;
	int32 m_SendMode;
	bool m_bShouldLingerOnClose;
	bool m_bIsListenSocket;
	bool m_bIsLANSocket;
	bool m_bHasPendingData;
	ESteamNetConnectionEnd m_ClosureReason;
	SteamNetworkingMessage_t* m_PendingData;

	class FSteamCoreSocketsSubsystem* m_SocketSubsystem;

	void SetLanOptions();

public:
	FSteamCoreSocket(ESocketType InSocketType, const FString& InSocketDescription, const FName& InSocketProtocol);
	virtual ~FSteamCoreSocket() override;

	virtual bool Close() override;

	virtual bool Bind(const FInternetAddr& Addr) override;
	virtual bool Connect(const FInternetAddr& Addr) override;
	virtual bool Listen(int32 MaxBacklog) override;

	virtual FSocket* Accept(const FString& InSocketDescription) override;
	virtual FSocket* Accept(FInternetAddr& OutAddr, const FString& InSocketDescription) override { return nullptr; }

	virtual bool SendTo(const uint8* Data, int32 Count, int32& BytesSent, const FInternetAddr& Destination) override;

	virtual bool Send(const uint8* Data, int32 Count, int32& BytesSent) override;
	virtual bool Recv(uint8* Data, int32 BufferSize, int32& BytesRead, ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None) override;

	virtual bool HasPendingData(uint32& PendingDataSize) override;
	virtual ESocketConnectionState GetConnectionState() override;

	virtual void GetAddress(FInternetAddr& OutAddr) override { OutAddr = m_BindAddress; }
	virtual bool GetPeerAddress(FInternetAddr& OutAddr) override;

	virtual bool SetNoDelay(bool bIsNoDelay = true) override;
	virtual bool SetLinger(bool bShouldLinger = true, int32 Timeout = 0) override;

	virtual bool SetSendBufferSize(int32 Size, int32& NewSize) override;
	virtual bool SetReceiveBufferSize(int32 Size, int32& NewSize) override;

	virtual int32 GetPortNo() override
	{
		return m_BindAddress.GetPort();
	}

	virtual bool Shutdown(ESocketShutdownMode Mode) override;
	virtual bool Wait(ESocketWaitConditions::Type Condition, FTimespan WaitTime) override;
	virtual bool WaitForPendingConnection(bool& bHasPendingConnection, const FTimespan& WaitTime) override;
	virtual bool SetReuseAddr(bool bAllowReuse = true) override;
	virtual bool SetRecvErr(bool bUseErrorQueue = true) override;
	virtual bool SetNonBlocking(bool bIsNonBlocking = true) override;
	virtual bool SetBroadcast(bool bAllowBroadcast = true) override;
	virtual bool JoinMulticastGroup(const FInternetAddr& GroupAddress) override;
	virtual bool JoinMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress) override;
	virtual bool LeaveMulticastGroup(const FInternetAddr& GroupAddress) override;
	virtual bool LeaveMulticastGroup(const FInternetAddr& GroupAddress, const FInternetAddr& InterfaceAddress);
	virtual bool SetMulticastLoopback(bool bLoopback) override;
	virtual bool SetMulticastTtl(uint8 TimeToLive) override;
	virtual bool SetMulticastInterface(const FInternetAddr& InterfaceAddress) override;

	// ~FSocket implementation

	bool RecvRaw(SteamNetworkingMessage_t*& Data, int32 MaxMessages, int32& MessagesRead, ESocketReceiveFlags::Type Flags = ESocketReceiveFlags::None);

	void SetClosureReason(ESteamNetConnectionEnd NewClosureReason) { m_ClosureReason = NewClosureReason; }
	void SetSendMode(int32 NewSendMode);
};
#endif
