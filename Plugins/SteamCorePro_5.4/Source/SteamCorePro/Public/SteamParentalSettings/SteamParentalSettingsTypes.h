/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamParentalSettings
*/

#pragma once

#include "CoreMinimal.h"
#include "SteamCorePro/SteamCoreProModule.h"
#include "SteamCorePro/Steam.h"
#include "SteamParentalSettingsTypes.generated.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Enums
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

UENUM(BlueprintType)
enum class ESteamParentalFeature : uint8
{
	k_EFeatureInvalid = 0,
	k_EFeatureStore = 1,
	k_EFeatureCommunity = 2,
	k_EFeatureProfile = 3,
	k_EFeatureFriends = 4,
	k_EFeatureNews = 5,
	k_EFeatureTrading = 6,
	k_EFeatureSettings = 7,
	k_EFeatureConsole = 8,
	k_EFeatureBrowser = 9,
	k_EFeatureParentalSetup = 10,
	k_EFeatureLibrary = 11,
	k_EFeatureTest = 12,
	k_EFeatureSiteLicense = 13,
	k_EFeatureKioskMode_Deprecated = 14,
	k_EFeatureBlockAlways = 15,
	k_EFeatureMax
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Structs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

USTRUCT(BlueprintType)
struct FSteamParentalSettingsChanged
{
	GENERATED_BODY()
public:
	FSteamParentalSettingsChanged() = default;

#if WITH_STEAMCORE
	FSteamParentalSettingsChanged(const SteamParentalSettingsChanged_t& data)
	{
	}
#endif
};