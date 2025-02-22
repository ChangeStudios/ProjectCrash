/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "IPAddress.h"
#include "SteamCoreSharedTypes.h"
#include "SteamCoreSocketsPrivate.h"
#include "SteamCoreSocketsPackage.h"
#include "SteamCoreSocketsTypes.h"
#include "Engine/EngineBaseTypes.h"

#if WITH_STEAMCORE
class FInternetAddrSteamCoreSockets : public FInternetAddr
{
PACKAGE_SCOPE:
	SteamNetworkingIdentity m_Addr;
	int32 m_P2PVirtualPort;
	FName m_ProtocolType;

public:
	FInternetAddrSteamCoreSockets(const FName RequestedProtocol = NAME_None)
		: m_P2PVirtualPort(0),
		  m_ProtocolType(RequestedProtocol)
	{
		m_Addr.Clear();
	}

	FInternetAddrSteamCoreSockets(const FInternetAddrSteamCoreSockets& In)
		: m_Addr(In.m_Addr),
		  m_P2PVirtualPort(In.m_P2PVirtualPort),
		  m_ProtocolType(In.m_ProtocolType)
	{
	}

	FInternetAddrSteamCoreSockets(const SteamNetworkingIdentity& NewAddress)
		: m_Addr(NewAddress),
		  m_P2PVirtualPort(0),
		  m_ProtocolType(NewAddress.GetIPAddr() == nullptr ? FNetworkProtocolTypes::SteamCoreSocketsP2P : FNetworkProtocolTypes::SteamCoreSocketsIP)
	{
	}

	FInternetAddrSteamCoreSockets(const SteamNetworkingIPAddr& IPAddr)
		: m_P2PVirtualPort(0),
		  m_ProtocolType(FNetworkProtocolTypes::SteamCoreSocketsIP)
	{
		m_Addr.SetIPAddr(IPAddr);
	}

	explicit FInternetAddrSteamCoreSockets(uint64& SteamID)
		: m_P2PVirtualPort(0),
		  m_ProtocolType(FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		m_Addr.SetSteamID64(SteamID);
	}

	virtual TArray<uint8> GetRawIp() const override;

	virtual void SetRawIp(const TArray<uint8>& RawAddr) override;

	virtual void SetIp(uint32 InAddr) override
	{
	}

	virtual void SetIp(const TCHAR* InAddr, bool& bIsValid) override;

	virtual void GetIp(uint32& OutAddr) const override
	{
	}

	virtual void SetPort(int32 InPort) override;
	virtual int32 GetPort() const override;

	virtual void SetPlatformPort(int32 InPort) override
	{
		m_P2PVirtualPort = static_cast<int16>(InPort);
	}

	virtual int32 GetPlatformPort() const override
	{
		return (int32)m_P2PVirtualPort;
	}

	virtual void SetAnyAddress() override;

	virtual void SetBroadcastAddress() override
	{
	}

	virtual void SetLoopbackAddress() override
	{
		m_Addr.SetLocalHost();
	}

	virtual FString ToString(bool bAppendPort) const override;

	virtual bool operator==(const FInternetAddr& Other) const override
	{
		const FInternetAddrSteamCoreSockets& SteamOther = (FInternetAddrSteamCoreSockets&)Other;
		return m_Addr == SteamOther.m_Addr;
	}

	virtual uint32 GetTypeHash() const override;
	virtual FName GetProtocolType() const override;

	virtual bool IsValid() const override
	{
		return !(m_Addr.IsInvalid());
	}

	operator const SteamNetworkingIPAddr() const
	{
		const SteamNetworkingIPAddr* IPAddr = m_Addr.GetIPAddr();
		if (IPAddr == nullptr)
		{
			SteamNetworkingIPAddr EmptyAddr;
			EmptyAddr.Clear();
			return EmptyAddr;
		}
		return *IPAddr;
	}

	operator const SteamNetworkingIdentity() const
	{
		return m_Addr;
	}

	virtual TSharedRef<FInternetAddr> Clone() const override
	{
		TSharedRef<FInternetAddrSteamCoreSockets> NewAddress = MakeShareable(new FInternetAddrSteamCoreSockets(m_ProtocolType));
		NewAddress->m_Addr = m_Addr;
		NewAddress->m_P2PVirtualPort = m_P2PVirtualPort;
		return NewAddress;
	}
};
#endif
