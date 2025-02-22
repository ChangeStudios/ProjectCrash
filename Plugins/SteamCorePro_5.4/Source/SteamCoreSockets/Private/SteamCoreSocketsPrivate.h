/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"

#ifndef STEAM_URL_PREFIX
#define STEAM_URL_PREFIX TEXT("steam.")
#endif

#if WITH_STEAMCORE
THIRD_PARTY_INCLUDES_START
#include "steam_api.h"
#include "steam_gameserver.h"
#include "isteamnetworkingsockets.h"
#include "isteamnetworkingutils.h"
#include "steamnetworkingtypes.h"
THIRD_PARTY_INCLUDES_END
#endif

#ifdef _MSC_VER
#define STEAM_SDK_IGNORE_REDUNDANCY_START \
		__pragma(warning(push)) \
		__pragma(warning(disable: 6287)) 

#define STEAM_SDK_IGNORE_REDUNDANCY_END \
		__pragma(warning(pop))
#else
#define STEAM_SDK_IGNORE_REDUNDANCY_START
#define STEAM_SDK_IGNORE_REDUNDANCY_END
#endif
