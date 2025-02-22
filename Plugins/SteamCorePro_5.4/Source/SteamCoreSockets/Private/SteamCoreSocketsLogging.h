/**
* Copyright (C) 2017-2024 eelDev AB
*/

#pragma once

#define LogSteamCoreDebug(format, ...) UE_LOG(LogSteamCoreSockets, Log, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVerbose(format, ...) UE_LOG(LogSteamCoreSockets, Verbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreVeryVerbose(format, ...) UE_LOG(LogSteamCoreSockets, VeryVerbose, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreError(format, ...) UE_LOG(LogSteamCoreSockets, Error, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
#define LogSteamCoreWarn(format, ...) UE_LOG(LogSteamCoreSockets, Warning, TEXT("[%s] " format), *FString(__FUNCTION__), ##__VA_ARGS__)
