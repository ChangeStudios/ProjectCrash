/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamParentalSettings
*/

#include "SteamParentalSettings/SteamParentalSettings.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProParentalSettings::USteamProParentalSettings()
{
}

USteamProParentalSettings::~USteamProParentalSettings()
{
}

USteamProParentalSettings* USteamProParentalSettings::GetSteamParentalSettings()
{
#if WITH_STEAMCORE
	return SteamParentalSettings() ? ThisClass::StaticClass()->GetDefaultObject<USteamProParentalSettings>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

bool USteamProParentalSettings::BIsParentalLockEnabled()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsParentalLockEnabled();
	}
#endif

	return bResult;
}

bool USteamProParentalSettings::BIsParentalLockLocked()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsParentalLockLocked();
	}
#endif

	return bResult;
}

bool USteamProParentalSettings::BIsAppBlocked(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsAppBlocked(AppID);
	}
#endif

	return bResult;
}

bool USteamProParentalSettings::BIsAppInBlockList(int32 AppID)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsAppInBlockList(AppID);
	}
#endif

	return bResult;
}

bool USteamProParentalSettings::BIsFeatureBlocked(ESteamParentalFeature Feature)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsFeatureBlocked(static_cast<EParentalFeature>(Feature));
	}
#endif

	return bResult;
}

bool USteamProParentalSettings::BIsFeatureInBlockList(ESteamParentalFeature Feature)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamParentalSettings())
	{
		bResult = SteamParentalSettings()->BIsFeatureInBlockList(static_cast<EParentalFeature>(Feature));
	}
#endif

	return bResult;
}
