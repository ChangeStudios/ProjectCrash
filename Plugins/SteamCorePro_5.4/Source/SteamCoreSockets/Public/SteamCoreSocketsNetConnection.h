/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "Engine/NetConnection.h"
#include "SteamCoreSocketsNetConnection.generated.h"

class FSocket;
class UNetDriver;
class FInternetAddr;
class FSteamCoreSocket;

UCLASS(transient, config=Engine)
class STEAMCORESOCKETS_API USteamCoreSocketsNetConnection : public UNetConnection
{
	GENERATED_BODY()

public:
	USteamCoreSocketsNetConnection() :
		m_ConnectionSocket(nullptr),
		m_bInConnectionlessHandshake(false)
	{
	}

	//~ Begin NetConnection Interface
	virtual void CleanUp() override;
	virtual void InitBase(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitRemoteConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, const FInternetAddr& InRemoteAddr, 
		EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void InitLocalConnection(UNetDriver* InDriver, FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBits, FOutPacketTraits& Traits) override;
	FString LowLevelGetRemoteAddress(bool bAppendPort=false) override;
	FString LowLevelDescribe() override;
	//~ End NetConnection Interface

private:
	const FSteamCoreSocket* GetRawSocket() const { return m_ConnectionSocket; }
	void HandleRecvMessage(void* InData, int32 SizeOfData, const FInternetAddr* InFormattedAddress);
	void FlagForHandshake() { m_bInConnectionlessHandshake = true; }
	void ClearSocket() { m_ConnectionSocket = nullptr; }

	FSteamCoreSocket* m_ConnectionSocket;
	bool m_bInConnectionlessHandshake;

	friend class USteamCoreSocketsNetDriver;
};
