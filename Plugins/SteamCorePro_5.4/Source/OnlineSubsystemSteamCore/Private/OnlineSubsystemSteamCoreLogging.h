/**
* Copyright (C) 2017-2024 eelDev AB
*/

#pragma once

#define LogSteamCoreDebug(format, ...) UE_LOG(LogOnlineSubsystemSteamCore, Log, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVerbose(format, ...) UE_LOG(LogOnlineSubsystemSteamCore, Verbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVeryVerbose(format, ...) UE_LOG(LogOnlineSubsystemSteamCore, VeryVerbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreError(format, ...) UE_LOG(LogOnlineSubsystemSteamCore, Error, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreWarn(format, ...) UE_LOG(LogOnlineSubsystemSteamCore, Warning, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)