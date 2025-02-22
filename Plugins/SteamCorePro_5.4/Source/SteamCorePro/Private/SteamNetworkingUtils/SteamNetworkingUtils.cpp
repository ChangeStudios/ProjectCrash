/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamNetworkingUtils
*/

#include "SteamNetworkingUtils/SteamNetworkingUtils.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProNetworkingUtils::USteamProNetworkingUtils()
{
}

USteamProNetworkingUtils::~USteamProNetworkingUtils()
{
}

USteamProNetworkingUtils* USteamProNetworkingUtils::GetSteamNetworkingUtils()
{
#if WITH_STEAMCORE
	return SteamNetworkingUtils() ? ThisClass::StaticClass()->GetDefaultObject<USteamProNetworkingUtils>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void USteamProNetworkingUtils::InitRelayNetworkAccess()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamNetworkingUtils())
	{
		SteamNetworkingUtils()->InitRelayNetworkAccess();
	}
#endif
}

float USteamProNetworkingUtils::GetLocalPingLocation(FSteamNetworkPingLocation& Result)
{
	LogSteamCoreVeryVerbose("");

	float FloatResult = 0.0f;

#if WITH_STEAMCORE
	if (SteamNetworkingUtils())
	{
		SteamNetworkPingLocation_t SteamNetworkPingLocation;
		FloatResult = SteamNetworkingUtils()->GetLocalPingLocation(SteamNetworkPingLocation);
		Result = SteamNetworkPingLocation;
	}
#endif

	return FloatResult;
}

int32 USteamProNetworkingUtils::EstimatePingTimeBetweenTwoLocations(const FSteamNetworkPingLocation& Location1, const FSteamNetworkPingLocation& Location2)
{
	LogSteamCoreVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamNetworkingUtils())
	{
		Result = SteamNetworkingUtils()->EstimatePingTimeBetweenTwoLocations(Location1, Location2);
	}
#endif
	
	return Result;
}

int32 USteamProNetworkingUtils::EstimatePingTimeFromLocalHost(const FSteamNetworkPingLocation& RemoteLocation)
{
	LogSteamCoreVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamNetworkingUtils())
	{
		Result = SteamNetworkingUtils()->EstimatePingTimeFromLocalHost(RemoteLocation);
	}
#endif
	
	return Result;
}

void USteamProNetworkingUtils::ConvertPingLocationToString(const FSteamNetworkPingLocation& Location, FString& OutString)
{
	LogSteamCoreVerbose("");

	OutString.Empty();

#if WITH_STEAMCORE
	TArray<char> DataArray;
	DataArray.SetNum(k_cchMaxSteamNetworkingPingLocationString);

	SteamNetworkingUtils()->ConvertPingLocationToString(Location, DataArray.GetData(), DataArray.Num());
	OutString = UTF8_TO_TCHAR(DataArray.GetData());
#endif
}

bool USteamProNetworkingUtils::ParsePingLocationString(FString String, FSteamNetworkPingLocation& OutResult)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	SteamNetworkPingLocation_t Data;
	bResult = SteamNetworkingUtils()->ParsePingLocationString(TCHAR_TO_UTF8(*String), Data);
	OutResult = Data;
#endif

	return bResult;
}

bool USteamProNetworkingUtils::CheckPingDataUpToDate(float MaxAgeSeconds)
{
	LogSteamCoreVerbose("");

	bool bResult = false;
	
#if WITH_STEAMCORE
	bResult = SteamNetworkingUtils()->CheckPingDataUpToDate(MaxAgeSeconds);
#endif
	
	return bResult;
}
