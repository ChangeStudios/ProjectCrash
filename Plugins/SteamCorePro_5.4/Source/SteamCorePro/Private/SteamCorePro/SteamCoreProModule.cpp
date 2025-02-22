/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#include "SteamCorePro/SteamCoreProModule.h"
#include "SteamCorePro/Steam.h"
#include "SteamCorePro/SteamCoreProAsync.h"
#include "SteamCoreProPluginPrivatePCH.h"
#include <Interfaces/IPluginManager.h>
#include "OnlineSubsystemSteamCore.h"

#define LOCTEXT_NAMESPACE "FSteamCoreProModule"
#define STEAM_SDK_ROOT_PATH TEXT("Binaries/ThirdParty/Steamworks")

DEFINE_LOG_CATEGORY(LogSteamCorePro);

FString FSteamCoreProModule::s_PluginName;
FString FSteamCoreProModule::s_PluginVersion;

void FSteamCoreProModule::StartupModule()
{
	TSharedPtr<IPlugin> PluginPtr = IPluginManager::Get().FindPlugin("SteamCorePro");

	if (PluginPtr)
	{
		s_PluginName = "SteamCorePro";
		s_PluginVersion = PluginPtr->GetDescriptor().VersionName;
	}

	UE_LOG(LogTemp, Log, TEXT("--------------------------------------------------------------------------------"));
	UE_LOG(LogTemp, Log, TEXT("Using %s Version: %s"), *s_PluginName, *s_PluginVersion);
	UE_LOG(LogTemp, Log, TEXT("--------------------------------------------------------------------------------"));
}

void FSteamCoreProModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSteamCoreProModule, SteamCorePro)

void USteamCoreInterface::QueueAsyncTask(FOnlineAsyncTask* AsyncTask)
{
#if WITH_STEAMCORE
	FOnlineSubsystemSteamCore* SteamCoreOSS = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	
	if (SteamCoreOSS)
	{
		SteamCoreOSS->QueueAsyncTask(AsyncTask);
	}
#endif
}

void USteamCoreProSubsystem::QueueAsyncTask(FOnlineAsyncTask* AsyncTask)
{
#if WITH_STEAMCORE
	FOnlineSubsystemSteamCore* SteamCoreOSS = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	
	if (SteamCoreOSS)
	{
		SteamCoreOSS->QueueAsyncTask(AsyncTask);
	}
#endif
}

USteamCoreProSubsystem* USteamCoreProSubsystem::Get()
{
	return ThisClass::StaticClass()->GetDefaultObject<USteamCoreProSubsystem>();
}
