/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "IPAddressSteamCore.h"
#include "SteamCoreSharedTypes.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#if WITH_STEAMCORE
TArray<uint8> FInternetAddrSteamCore::GetRawIp() const
{
	TArray<uint8> RawAddressArray;
	const uint8* SteamIdWalk = m_SteamId->GetBytes();
	while (RawAddressArray.Num() < m_SteamId->GetSize())
	{
		RawAddressArray.Add(*SteamIdWalk);
		++SteamIdWalk;
	}

#if PLATFORM_LITTLE_ENDIAN
	Algo::Reverse(RawAddressArray);
#endif

	return RawAddressArray;
}

void FInternetAddrSteamCore::SetRawIp(const TArray<uint8>& RawAddr)
{
	uint64 NewSteamId = 0;
	TArray<uint8> WorkingArray = RawAddr;
#if PLATFORM_LITTLE_ENDIAN
	Algo::Reverse(WorkingArray);
#endif

	for (int32 i = 0; i < WorkingArray.Num(); ++i)
	{
		NewSteamId |= static_cast<uint64>(WorkingArray[i]) << (i * 8);
	}

	m_SteamId = FUniqueNetIdSteam::Create(NewSteamId);
}

void FInternetAddrSteamCore::SetIp(const TCHAR* InAddr, bool& bIsValid)
{
	bIsValid = false;
	FString InAddrStr(InAddr);

	FString SteamIPAddrStr;
	if (InAddrStr.StartsWith(STEAM_URL_PREFIX))
	{
		SteamIPAddrStr = InAddrStr.Mid(UE_ARRAY_COUNT(STEAM_URL_PREFIX) - 1);
	}
	else
	{
		SteamIPAddrStr = InAddrStr;
	}

	FString SteamIPStr, SteamChannelStr;
	if (SteamIPAddrStr.Split(":", &SteamIPStr, &SteamChannelStr, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		const uint64 Id = FCString::Atoi64(*SteamIPStr);
		if (Id != 0)
		{
			m_SteamId = FUniqueNetIdSteam::Create(Id);
			const int32 Channel = FCString::Atoi(*SteamChannelStr);
			if (Channel != 0 || SteamChannelStr == "0")
			{
				m_SteamChannel = Channel;
				bIsValid = true;
			}
		}
	}
	else
	{
		const uint64 Id = FCString::Atoi64(*SteamIPAddrStr);
		if (Id != 0)
		{
			m_SteamId = FUniqueNetIdSteam::Create(Id);
			bIsValid = true;
		}

		m_SteamChannel = 0;
	}

	bIsValid = bIsValid && m_SteamId->IsValid();
}

FName FInternetAddrSteamCore::GetProtocolType() const
{
	return FNetworkProtocolTypes::SteamCore;
}
#endif