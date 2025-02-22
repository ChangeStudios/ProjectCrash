/**
* Copyright (C) 2017-2024 eelDev AB
*/

#pragma once

#define LogSteamCoreDebug(format, ...) UE_LOG(LogSteamCoreShared, Log, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVerbose(format, ...) UE_LOG(LogSteamCoreShared, Verbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVeryVerbose(format, ...) UE_LOG(LogSteamCoreShared, VeryVerbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreError(format, ...) UE_LOG(LogSteamCoreShared, Error, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreWarn(format, ...) UE_LOG(LogSteamCoreShared, Warning, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
