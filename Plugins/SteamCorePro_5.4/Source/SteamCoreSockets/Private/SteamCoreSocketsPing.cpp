/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocketsPing.h"
#include "EngineLogs.h"
#include "SteamCoreSocketsPrivate.h"
#include "SteamCoreSocketsSubsystem.h"

#if WITH_STEAMCORE
bool FSteamCoreSocketsPing::IsUsingP2PRelays() const
{
	if (m_SocketSub)
	{
		return m_SocketSub->IsUsingRelayNetwork();
	}

	return false;
}

FString FSteamCoreSocketsPing::GetHostPingData() const
{
	if (!IsUsingP2PRelays())
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: GetHostPingData was called but relays are disabled! This is invalid!"));
		return TEXT("");
	}

	if (!SteamNetworkingUtils() || !m_SocketSub)
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Could not get host ping data as SteamNetworking is not available."));
		return TEXT("");
	}

	SteamNetworkPingLocation_t PingData;
	const float PingDataAge = SteamNetworkingUtils()->GetLocalPingLocation(PingData);

	if (PingDataAge == -1)
	{
		UE_LOG(LogNet, Log, TEXT("SteamCoreSockets: No ping data is available at this time"));
		return TEXT("");
	}

	ANSICHAR ConversionBuffer[k_cchMaxSteamNetworkingPingLocationString];
	FMemory::Memzero(ConversionBuffer);
	SteamNetworkingUtils()->ConvertPingLocationToString(PingData, ConversionBuffer, k_cchMaxSteamNetworkingPingLocationString);

	return FString(ANSI_TO_TCHAR(ConversionBuffer));
}

int32 FSteamCoreSocketsPing::GetPingFromHostData(const FString& HostPingStr) const
{
	if (!SteamNetworkingUtils() || HostPingStr.IsEmpty() || !IsUsingP2PRelays())
	{
		UE_LOG(LogNet, Warning, TEXT("SteamCoreSockets: Could not determine ping data as SteamNetworking is unavailable."));
		return -1;
	}

	SteamNetworkPingLocation_t HostPingData;
	if (SteamNetworkingUtils()->ParsePingLocationString(TCHAR_TO_ANSI(*HostPingStr), HostPingData))
	{
		const int32 PingValue = SteamNetworkingUtils()->EstimatePingTimeFromLocalHost(HostPingData);
		if (PingValue == -1)
		{
			return -1;
		}

		return PingValue;
	}
	else
	{
		UE_LOG(LogNet, Log, TEXT("SteamCoreSockets: Could not parse the host location in order to determine ping."));
		return -1;
	}
}


bool FSteamCoreSocketsPing::IsRecalculatingPing() const
{
	if (!SteamNetworkingUtils() && IsUsingP2PRelays())
	{
		return false;
	}

	const ESteamNetworkingAvailability NetworkStatus = SteamNetworkingUtils()->GetRelayNetworkStatus(nullptr);
	return NetworkStatus == k_ESteamNetworkingAvailability_Retrying || NetworkStatus == k_ESteamNetworkingAvailability_Waiting || NetworkStatus == k_ESteamNetworkingAvailability_Attempting;
}
#endif
