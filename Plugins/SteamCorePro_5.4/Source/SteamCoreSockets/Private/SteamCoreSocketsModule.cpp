/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocketsModule.h"
#include "SteamCoreSocketsSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteamCore.h"
#include "SteamCoreSocketsTypes.h"
#include "SocketSubsystemModule.h"

#include "SteamCoreSharedModule.h"
#include "SteamCoreSharedTypes.h"

IMPLEMENT_MODULE(FSteamCoreSocketsModule, SteamCoreSockets);

DEFINE_LOG_CATEGORY(LogSteamCoreSockets);

void FSteamCoreSocketsModule::StartupModule()
{
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSub = static_cast<const FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	const FSteamCoreSharedModule& SharedModule = FSteamCoreSharedModule::Get();
	const bool bIsNotEditor = (IsRunningDedicatedServer() || IsRunningGame());
	const bool bSteamOSSEnabled = (SteamSub && SteamSub->IsEnabled());

	if (SharedModule.AreSteamDllsLoaded() && bIsNotEditor && bSteamOSSEnabled)
	{
		bool bOverrideSocketSubsystem = false;

		if (GConfig)
		{
			GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bUseSteamNetworking"), bOverrideSocketSubsystem, GEngineIni);
		}

		if (bOverrideSocketSubsystem)
		{
			UE_LOG(LogSockets, Log, TEXT("bUseSteamNetworking is TRUE. Overriding default socket subsystem with SteamCoreSockets"));
			
			FSteamCoreSocketsSubsystem* SocketSubsystem = FSteamCoreSocketsSubsystem::Create();
			FString Error;
			if (SocketSubsystem->Init(Error))
			{
				m_bEnabled = true;

				FSocketSubsystemModule& SSS = FModuleManager::LoadModuleChecked<FSocketSubsystemModule>("Sockets");
				SSS.RegisterSocketSubsystem(STEAMCORE_SOCKETS_SUBSYSTEM, SocketSubsystem, bOverrideSocketSubsystem);
			}
			else
			{
				UE_LOG(LogSockets, Error, TEXT("SteamCoreSockets: Could not initialize SteamCoreSockets, got error: %s"), *Error);
				FSteamCoreSocketsSubsystem::Destroy();
			}
		}

	}
	else if (!bSteamOSSEnabled)
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Disabled due to no Steam OSS running."));
	}
	else if (bIsNotEditor)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Steam SDK libraries not present at %s or failed to load!"), *SharedModule.GetSteamModulePath());
	}
	else
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets is disabled when running in the editor."));
	}
#endif
}

void FSteamCoreSocketsModule::ShutdownModule()
{
#if WITH_STEAMCORE
	const FModuleManager& ModuleManager = FModuleManager::Get();

	if (ModuleManager.IsModuleLoaded("Sockets"))
	{
		FSocketSubsystemModule& SSS = FModuleManager::GetModuleChecked<FSocketSubsystemModule>("Sockets");
		SSS.UnregisterSocketSubsystem(STEAMCORE_SOCKETS_SUBSYSTEM);
	}
	FSteamCoreSocketsSubsystem::Destroy();
#endif
}
