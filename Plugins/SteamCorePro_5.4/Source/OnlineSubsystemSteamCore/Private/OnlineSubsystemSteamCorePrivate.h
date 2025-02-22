/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteamCoreLogging.h"

#define INVALID_INDEX -1
#define STEAM_URL_PREFIX TEXT("steam.")
#define STEAMAPPIDFILENAME TEXT("steam_appid.txt")
#undef ONLINE_LOG_PREFIX
#define ONLINE_LOG_PREFIX TEXT("STEAM: ")

#if WITH_STEAMCORE
THIRD_PARTY_INCLUDES_START
#include "steam_api.h"
#include "steam_gameserver.h"
#include "isteamnetworkingsockets.h"
#include "steamnetworkingtypes.h"
THIRD_PARTY_INCLUDES_END
#endif