/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "IPAddressSteamCoreSockets.h"
#include "SteamCoreSocketsPrivate.h"
#include "Misc/EngineVersionComparison.h"
#include "Algo/Reverse.h"

#if WITH_STEAMCORE
TArray<uint8> FInternetAddrSteamCoreSockets::GetRawIp() const
{
	TArray<uint8> RawAddressArray;

	if (m_Addr.m_eType == k_ESteamNetworkingIdentityType_SteamID)
	{
		uint64 SteamIDNum = m_Addr.GetSteamID64();
		const uint8* SteamIdWalk = reinterpret_cast<uint8*>(&SteamIDNum);
		while (RawAddressArray.Num() < sizeof(uint64))
		{
			RawAddressArray.Add(*SteamIdWalk);
			++SteamIdWalk;
		}

#if PLATFORM_LITTLE_ENDIAN
		Algo::Reverse(RawAddressArray);
#endif

		RawAddressArray.EmplaceAt(0, k_ESteamNetworkingIdentityType_SteamID);
	}
	else if (m_Addr.m_eType == k_ESteamNetworkingIdentityType_IPAddress && m_Addr.GetIPAddr())
	{
		const SteamNetworkingIPAddr* RawSteamIP = m_Addr.GetIPAddr();
		RawAddressArray.Add(k_ESteamNetworkingIdentityType_IPAddress);
		for (int32 i = 0; i < UE_ARRAY_COUNT(RawSteamIP->m_ipv6); ++i)
		{
			RawAddressArray.Add(RawSteamIP->m_ipv6[i]);
		}
	}

	return RawAddressArray;
}

void FInternetAddrSteamCoreSockets::SetRawIp(const TArray<uint8>& RawAddr)
{
	if (RawAddr.Num() <= 1)
	{
		return;
	}

	m_Addr.Clear();
	const uint8 ArrayType = RawAddr[0];

	if (ArrayType == k_ESteamNetworkingIdentityType_SteamID)
	{
		TArray<uint8> WorkingArray = RawAddr;
		WorkingArray.RemoveAt(0);

#if PLATFORM_LITTLE_ENDIAN
		Algo::Reverse(WorkingArray);
#endif
		uint64 NewSteamId = 0;
		for (int32 i = 0; i < WorkingArray.Num(); ++i)
		{
			NewSteamId |= static_cast<uint64>(WorkingArray[i]) << (i * 8);
		}

		m_Addr.SetSteamID64(NewSteamId);
		m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsP2P;
	}
	else if (ArrayType == k_ESteamNetworkingIdentityType_IPAddress)
	{
		SteamNetworkingIPAddr NewAddr;
		NewAddr.Clear();

		for (int32 i = 1; i < RawAddr.Num(); ++i)
		{
			NewAddr.m_ipv6[i - 1] = RawAddr[i];
		}

		m_Addr.SetIPAddr(NewAddr);
		m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsIP;
	}
}

void FInternetAddrSteamCoreSockets::SetIp(const TCHAR* InAddr, bool& bIsValid)
{
	FString InAddrStr(InAddr);

	if (InAddrStr.StartsWith(STEAM_URL_PREFIX) || InAddrStr.IsNumeric())
	{
		InAddrStr.RemoveFromStart(STEAM_URL_PREFIX);
		FString SteamIPStr, SteamChannelStr;

		if (InAddrStr.Split(":", &SteamIPStr, &SteamChannelStr, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
		{
			const uint64 Id = FCString::Atoi64(*SteamIPStr);
			if (Id != 0)
			{
				m_Addr.SetSteamID64(Id);
				const int32 Channel = FCString::Atoi(*SteamChannelStr);
				if (Channel != 0 || SteamChannelStr == "0")
				{
					m_P2PVirtualPort = Channel;
					bIsValid = true;
				}
				m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsP2P;
			}
			else
			{
				bIsValid = false;
				return;
			}
		}
		else
		{
			m_Addr.SetSteamID64(FCString::Atoi64(*InAddrStr));
			m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsP2P;
			bIsValid = true;
			return;
		}
	}
	else if (SteamNetworkingUtils())
	{
		SteamNetworkingIPAddr NewAddress;
		bIsValid = NewAddress.ParseString(TCHAR_TO_ANSI(InAddr));
		m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsIP;
		m_Addr.SetIPAddr(NewAddress);
	}
}

void FInternetAddrSteamCoreSockets::SetPort(int32 InPort)
{
	if (GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		m_P2PVirtualPort = InPort;
	}
	else
	{
		SteamNetworkingIPAddr NewAddressInfo;
		const SteamNetworkingIPAddr* InternalAddress = m_Addr.GetIPAddr();
		FMemory::Memcpy(NewAddressInfo, *InternalAddress);
		NewAddressInfo.m_port = InPort;
		m_Addr.SetIPAddr(NewAddressInfo);
	}
}

int32 FInternetAddrSteamCoreSockets::GetPort() const
{
	if (GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
		return m_P2PVirtualPort;
	}
	else if (const SteamNetworkingIPAddr* InternalAddress = m_Addr.GetIPAddr())
	{
		return InternalAddress->m_port;
	}

	return 0;
}

void FInternetAddrSteamCoreSockets::SetAnyAddress()
{
	m_Addr.Clear();
	SteamNetworkingIPAddr NewAddress;
	NewAddress.Clear();
	m_ProtocolType = FNetworkProtocolTypes::SteamCoreSocketsIP;
	m_Addr.SetIPAddr(NewAddress);
}

FString FInternetAddrSteamCoreSockets::ToString(bool bAppendPort) const
{
	if (!m_Addr.IsInvalid())
	{
		if (GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
		{
			FString BaseResult = FString::Printf(TEXT("%llu"), m_Addr.GetSteamID64());
			if (bAppendPort)
			{
				BaseResult += FString::Printf(TEXT(":%d"), m_P2PVirtualPort);
			}
			return BaseResult;
		}

		const SteamNetworkingIPAddr* InternalAddress = m_Addr.GetIPAddr();
		if (InternalAddress && SteamNetworkingUtils())
		{
			ANSICHAR StrBuffer[SteamNetworkingIPAddr::k_cchMaxString];
			FMemory::Memzero(StrBuffer);
			InternalAddress->ToString(StrBuffer, SteamNetworkingIPAddr::k_cchMaxString, bAppendPort);

			return ANSI_TO_TCHAR(StrBuffer);
		}
	}

	return TEXT("Invalid");
}

uint32 FInternetAddrSteamCoreSockets::GetTypeHash() const
{
	if (m_Addr.IsInvalid())
	{
		if (m_P2PVirtualPort == 0)
		{
			return 0;
		}
		else
		{
#if UE_VERSION_OLDER_THAN(5,2,0)
			return ::GetTypeHash(m_P2PVirtualPort);
#else
			return GetTypeHashHelper(m_P2PVirtualPort);
#endif
		}
	}

	if (GetProtocolType() == FNetworkProtocolTypes::SteamCoreSocketsP2P)
	{
#if UE_VERSION_OLDER_THAN(5,2,0)
		return ::GetTypeHash(m_Addr.GetSteamID64());
#else
		return GetTypeHashHelper(m_Addr.GetSteamID64());
#endif
	}
	
#if UE_VERSION_OLDER_THAN(5,2,0)
	return ::GetTypeHash(ToString(true));
#else
	return GetTypeHashHelper(ToString(true));
#endif
}

FName FInternetAddrSteamCoreSockets::GetProtocolType() const
{
	if (m_Addr.GetIPAddr() == nullptr)
	{
		return FNetworkProtocolTypes::SteamCoreSocketsP2P;
	}

	return FNetworkProtocolTypes::SteamCoreSocketsIP;
}
#endif