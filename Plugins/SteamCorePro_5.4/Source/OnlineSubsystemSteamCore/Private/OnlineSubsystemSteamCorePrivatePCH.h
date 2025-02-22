/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#pragma once

#include "Misc/EngineVersionComparison.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Serialization/BufferArchive.h"
#include "Interfaces/IPluginManager.h"
#include "Stats/Stats.h"
#include "Algo/Reverse.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Misc/ConfigCacheIni.h"
#include "PacketHandler.h"
#include "Engine/World.h"
#include "OnlineSubsystemUtils.h"
#include "UObject/NameTypes.h"
#include "Misc/CommandLine.h"
#if UE_VERSION_NEWER_THAN(5,1,0) 
#include "Online/OnlineSessionNames.h"
#endif
#if UE_VERSION_NEWER_THAN(5,0,3) 
#include "Online/LANBeacon.h"
#else
#include "LANBeacon.h"
#endif

#include "OnlineSubsystemSteamCorePrivate.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "OnlineSubsystemSteamCoreLogging.h"
#include "OnlineSubsystemSteamCoreModule.h"
#include "TaskManager/OnlineAsyncTaskManagerSteamCore.h"
#include "Sessions/OnlineSessionInterfaceSteamCore.h"
#include "Identity/OnlineIdentityInterfaceSteamCore.h"
#include "Presence/OnlinePresenceInterfaceSteamCore.h"
#include "Friends/OnlineFriendsInterfaceSteamCore.h"
#include "SharedCloud/OnlineUserCloudInterfaceSteamCore.h"
#include "SharedCloud/OnlineSharedCloudInterfaceSteamCore.h"
#include "Leaderboards/OnlineLeaderboardInterfaceSteamCore.h"
#include "Voice/VoiceInterfaceSteamCore.h"
#include "ExternalUI/OnlineExternalUIInterfaceSteamCore.h"
#include "Achievements/OnlineAchievementsInterfaceSteamCore.h"
#include "Auth/OnlineAuthInterfaceSteamCore.h"
#include "Auth/OnlineAuthInterfaceUtilsSteamCore.h"
#include "OnlineEncryptedAppTicketInterfaceSteamCore.h"
#include "OnlinePingInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "Leaderboards/OnlineLeaderboardInterfaceSteamCore.h"
#include "Sessions/OnlineSessionAsyncServerSteamCore.h"
#include "Sessions/OnlineSessionAsyncLobbySteamCore.h"
#include "OnlineSubsystemSteamCoreUtilities.h"
#include "SteamCoreSharedModule.h"
#include "Sessions/OnlineSessionTypesSteamCore.h"
#include "Sockets/SocketSubsystemSteamCore.h"
#include "Net/NboSerializerSteamCore.h"
#include "Sockets/SocketsSteamCore.h"
#include "SocketSubsystemModule.h"
#include "Net/SteamCoreNetConnection.h"
#include "SteamCoreNetDriver.h"
#include "Misc/ScopeLock.h"