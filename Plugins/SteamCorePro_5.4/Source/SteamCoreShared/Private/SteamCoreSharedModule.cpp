/**
* Copyright (C) 2017-2024 eelDev AB
*/

#include "SteamCoreSharedModule.h"
#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ConfigCacheIni.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Interfaces/IPluginManager.h"
#include "SteamCoreSharedLogging.h"
#include "Misc/MessageDialog.h"
#include "HAL/IConsoleManager.h"

#if WITH_STEAMCORE
THIRD_PARTY_INCLUDES_START
#include "steam_api.h"
#include "steam_gameserver.h"
THIRD_PARTY_INCLUDES_END
#endif

IMPLEMENT_MODULE(FSteamCoreSharedModule, SteamCoreShared);

DEFINE_LOG_CATEGORY(LogSteamCoreShared);

static void __cdecl SteamworksWarningMessageHook(int Severity, const char* Message)
{
	LogSteamCoreWarn("Steamworks SDK: %s", UTF8_TO_TCHAR(Message));

	switch (Severity)
	{
	case 0:
		LogSteamCoreVerbose("Steamworks SDK: %s", UTF8_TO_TCHAR(Message));
		break;
	case 1:
		LogSteamCoreWarn("Steamworks SDK: %s", UTF8_TO_TCHAR(Message));
		break;
	}
}

FString FSteamCoreSharedModule::GetSteamModulePath() const
{
	const FString BaseDir = IPluginManager::Get().FindPlugin("SteamCorePro")->GetBaseDir();
	FString STEAM_LIBRARY_PATH(TEXT("Source/ThirdParty/SteamLibrary/redistributable_bin"));
	FString STEAM_SDK_ROOT_PATH = FPaths::Combine(BaseDir, STEAM_LIBRARY_PATH);

#if PLATFORM_WINDOWS

#if PLATFORM_64BITS
	return STEAM_SDK_ROOT_PATH / TEXT("Win64/");
#else
		return STEAM_SDK_ROOT_PATH / TEXT("Win32/");
#endif	//PLATFORM_64BITS

#elif PLATFORM_LINUX

#if PLATFORM_64BITS
		return STEAM_SDK_ROOT_PATH / TEXT("x86_64-unknown-linux-gnu/");
#else
		return STEAM_SDK_ROOT_PATH / TEXT("i686-unknown-linux-gnu/");
#endif	//PLATFORM_64BITS

#elif PLATFORM_MAC
	return STEAM_SDK_ROOT_PATH / TEXT("osx/");
#else

	return FString();

#endif	//PLATFORM_WINDOWS
}

bool FSteamCoreSharedModule::CanLoadClientDllsOnServer() const
{
#if PLATFORM_WINDOWS
	if (IsRunningDedicatedServer())
	{
		return true;
	}
#endif
	return false;
}

void FSteamCoreSharedModule::StartupModule()
{
	LoadSteamModules();

#if WITH_STEAMCORE
	if (SteamUtils())
	{
		SteamUtils()->SetWarningMessageHook(SteamworksWarningMessageHook);
	}

	bool bOverrideMinHandshakeVersion = true;
	
	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideMinHandshakeVersion"), bOverrideMinHandshakeVersion, GEngineIni))
	{
		bOverrideMinHandshakeVersion = true;
	}

	if (bOverrideMinHandshakeVersion)
	{
		int32 OverrideMinHandshakeVersion = 0;
		if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("MinHandshakeVersion"), OverrideMinHandshakeVersion, GEngineIni))
		{
			OverrideMinHandshakeVersion = 0;
		}
		
		static const auto MinHandshakeVersion = IConsoleManager::Get().FindConsoleVariable(TEXT("net.MinHandshakeVersion"));
		if (MinHandshakeVersion)
		{
			if (OverrideMinHandshakeVersion >= 0)
			{
				MinHandshakeVersion->Set(OverrideMinHandshakeVersion, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Log, TEXT("net.MinHandshakeVersion: %i"), MinHandshakeVersion->GetInt());
			}
		}
	}

	bool bOverrideCurrentHandshakeVersion = true;
	
	if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bOverrideCurrentHandshakeVersion"), bOverrideCurrentHandshakeVersion, GEngineIni))
	{
		bOverrideCurrentHandshakeVersion = true;
	}

	if (bOverrideCurrentHandshakeVersion)
	{
		int32 OverrideCurrentHandshakeVersion = 0;
		if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("CurrentHandshakeVersion"), OverrideCurrentHandshakeVersion, GEngineIni))
		{
			OverrideCurrentHandshakeVersion = 0;
		}
		
		static const auto CurrentHandshakeVersion = IConsoleManager::Get().FindConsoleVariable(TEXT("net.CurrentHandshakeVersion"));
		if (CurrentHandshakeVersion)
		{
			if (OverrideCurrentHandshakeVersion >= 0)
			{
				CurrentHandshakeVersion->Set(OverrideCurrentHandshakeVersion, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Log, TEXT("net.CurrentHandshakeVersion: %i"), CurrentHandshakeVersion->GetInt());
			}
		}
	}
	
#endif

	auto& PluginManager = IPluginManager::Get();
	
	if (const TSharedPtr<IPlugin> OnlineSubsystemSteamPtr = PluginManager.FindPlugin("OnlineSubsystemSteam"))
	{
		if (OnlineSubsystemSteamPtr->IsEnabled())
		{
			UE_LOG(LogTemp, Error, TEXT("OnlineSubsystemSteam Plugin is ENABLED. You have to disable this plugin otherwise the SteamCore plugin will not function properly."));
#if WITH_EDITOR
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("OnlineSubsystemSteam Plugin is ENABLED. You have to disable this plugin otherwise the SteamCore plugin will not function properly."));
#endif
		}
	}

	if (const TSharedPtr<IPlugin> SteamSharedPtr = PluginManager.FindPlugin("SteamShared"))
	{
		if (SteamSharedPtr->IsEnabled())
		{
			UE_LOG(LogTemp, Error, TEXT("SteamShared Plugin is ENABLED. You have to disable this plugin otherwise the SteamCore plugin will not function properly."));
#if WITH_EDITOR
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("SteamShared Plugin is ENABLED. You have to disable this plugin otherwise the SteamCore plugin will not function properly."));
#endif
		}
	}
}

void FSteamCoreSharedModule::ShutdownModule()
{
	if (AreSteamDllsLoaded())
	{
		if (m_SteamClientObserver.IsValid())
		{
			const uint32 NumSharedReferences = m_SteamClientObserver.Pin().GetSharedReferenceCount();
			LogSteamCoreWarn("There are still %d additional Steam instances in use. These must be shutdown before unloading the module!", NumSharedReferences - 1);
		}
		m_SteamClientObserver.Reset();

		if (m_SteamServerObserver.IsValid())
		{
			const uint32 NumSharedReferences = m_SteamServerObserver.Pin().GetSharedReferenceCount();
			LogSteamCoreWarn("There are still %d additional Steam server instances in use. These must be shutdown before unloading the module!", NumSharedReferences - 1);
		}
		m_SteamServerObserver.Reset();
	}

	UnloadSteamModules();
}

TSharedPtr<FSteamCoreClientInstanceHandler> FSteamCoreSharedModule::ObtainSteamClientInstanceHandle()
{
	if (m_SteamClientObserver.IsValid())
	{
		return m_SteamClientObserver.Pin();
	}
	else
	{
		const TSharedPtr<FSteamCoreClientInstanceHandler> BaseInstance = MakeShareable(new FSteamCoreClientInstanceHandler(this));
		m_SteamClientObserver = BaseInstance;
		if (BaseInstance->IsInitialized())
		{
			return m_SteamClientObserver.Pin();
		}
		else
		{
			m_SteamClientObserver = nullptr;
		}
	}

	return nullptr;
}

TSharedPtr<FSteamCoreServerInstanceHandler> FSteamCoreSharedModule::ObtainSteamServerInstanceHandle()
{
	if (m_SteamServerObserver.IsValid())
	{
		return m_SteamServerObserver.Pin();
	}
	else
	{
		const TSharedPtr<FSteamCoreServerInstanceHandler> BaseInstance = MakeShareable(new FSteamCoreServerInstanceHandler(this));
		m_SteamServerObserver = BaseInstance;
		if (BaseInstance->IsInitialized())
		{
			return m_SteamServerObserver.Pin();
		}
		else
		{
			m_SteamServerObserver = nullptr;
		}
	}

	return nullptr;
}

bool FSteamCoreSharedModule::AreSteamDllsLoaded() const
{
	bool bLoadedClientDll = true;
	bool bLoadedServerDll = true;

#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
	bLoadedClientDll = (m_SteamDLLHandle != nullptr) ? true : false;
#endif // LOADING_STEAM_LIBRARIES_DYNAMICALLY
#if LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY
	bLoadedServerDll = IsRunningDedicatedServer() ? ((m_SteamServerDLLHandle != nullptr || !m_bForceLoadSteamClientDll) ? true : false) : true;
#endif // LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY

	return bLoadedClientDll && bLoadedServerDll;
}

void FSteamCoreSharedModule::LoadSteamModules()
{
	if (AreSteamDllsLoaded())
	{
		return;
	}

	LogSteamCoreVerbose("Loading Steam SDK");

#if PLATFORM_WINDOWS

#if PLATFORM_64BITS
	FString Suffix("64");
#else
	FString Suffix;
#endif // PLATFORM_64BITS

	FString RootSteamPath = GetSteamModulePath();
	FPlatformProcess::PushDllDirectory(*RootSteamPath);
	m_SteamDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + "steam_api" + Suffix + ".dll"));
	if (IsRunningDedicatedServer() && FCommandLine::IsInitialized() && FParse::Param(FCommandLine::Get(), TEXT("force_steamclient_link")))
	{
		FString SteamClientDLL("steamclient" + Suffix + ".dll"), SteamTierDLL("tier0_s" + Suffix + ".dll"), SteamVSTDDLL("vstdlib_s" + Suffix + ".dll");

		LogSteamCoreVerbose("Attempting to force linking the steam client dlls.");
		m_bForceLoadSteamClientDll = true;
		m_SteamServerDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + SteamClientDLL));
		if (!m_SteamServerDLLHandle)
		{
			LogSteamCoreError("Could not find the %s, %s and %s DLLs, make sure they are all located at %s! These dlls can be located in your Steam install directory.", *SteamClientDLL, *SteamTierDLL, *SteamVSTDDLL, *RootSteamPath);
		}
	}
	FPlatformProcess::PopDllDirectory(*RootSteamPath);
#elif PLATFORM_MAC || (PLATFORM_LINUX && LOADING_STEAM_LIBRARIES_DYNAMICALLY)

#if PLATFORM_MAC
	FString SteamModuleFileName("libsteam_api.dylib");
#else
	FString SteamModuleFileName("libsteam_api.so");
#endif // PLATFORM_MAC

	m_SteamDLLHandle = FPlatformProcess::GetDllHandle(*SteamModuleFileName);
	if (m_SteamDLLHandle == nullptr)
	{
		// try bundled one
#if PLATFORM_MAC
		if (FParse::Param(FCommandLine::Get(), TEXT("dllerrors")))
#endif // PLATFORM_MAC
		{
			LogSteamCoreWarn("Could not find system one, loading bundled %s.", *SteamModuleFileName);
		}
		FString RootSteamPath = GetSteamModulePath();
		m_SteamDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + SteamModuleFileName));
		LogSteamCoreWarn("Loading %s/%s", *RootSteamPath, *SteamModuleFileName);
	}

	if (m_SteamDLLHandle)
	{
		LogSteamCoreVerbose("Loaded %s at %p", *SteamModuleFileName, m_SteamDLLHandle);
	}
	else
	{
		LogSteamCoreWarn("Unable to load %s, Steam functionality will not work", *SteamModuleFileName);
		return;
	}


#elif PLATFORM_LINUX
	LogSteamCoreVerbose("libsteam_api.so is linked explicitly and should be already loaded.");
	return;
#endif // PLATFORM_WINDOWS
	LogSteamCoreVerbose("Steam SDK Loaded!");
}

void FSteamCoreSharedModule::UnloadSteamModules()
{
	if (!m_SteamClientObserver.IsValid() && !m_SteamServerObserver.IsValid())
	{
#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
		LogSteamCoreVerbose("Freeing the Steam Loaded Modules...");

		if (m_SteamDLLHandle != nullptr)
		{
			FPlatformProcess::FreeDllHandle(m_SteamDLLHandle);
			m_SteamDLLHandle = nullptr;
		}

		if (m_SteamServerDLLHandle != nullptr)
		{
			FPlatformProcess::FreeDllHandle(m_SteamServerDLLHandle);
			m_SteamServerDLLHandle = nullptr;
		}
#endif	//LOADING_STEAM_LIBRARIES_DYNAMICALLY
	}
}

FSteamCoreInstanceHandlerBase::FSteamCoreInstanceHandlerBase()
	: m_bInitialized(false)
{
	if (FParse::Value(FCommandLine::Get(), TEXT("Port="), m_GamePort) == false)
	{
		GConfig->GetInt(TEXT("URL"), TEXT("Port"), m_GamePort, GEngineIni);
	}
}

#if WITH_STEAMCORE
FString ESteamAPIInitResultToString(ESteamAPIInitResult Result)
{
	switch (Result) {
	case k_ESteamAPIInitResult_OK:
		return "k_ESteamAPIInitResult_OK";
		break;
	case k_ESteamAPIInitResult_FailedGeneric:
		return "k_ESteamAPIInitResult_FailedGeneric";
		break;
	case k_ESteamAPIInitResult_NoSteamClient:
		return "k_ESteamAPIInitResult_NoSteamClient";
		break;
	case k_ESteamAPIInitResult_VersionMismatch:
		return "k_ESteamAPIInitResult_VersionMismatch";
		break;
	}

	return "";
}
#endif

FSteamCoreClientInstanceHandler::FSteamCoreClientInstanceHandler(FSteamCoreSharedModule* SteamInitializer)
	: FSteamCoreInstanceHandlerBase()
{
#if WITH_STEAMCORE
	if (SteamInitializer != nullptr && SteamInitializer->AreSteamDllsLoaded())
	{
		SteamErrMsg ErrorMessage;
		ESteamAPIInitResult SteamAPIInitResult = SteamAPI_InitEx(&ErrorMessage);

		LogSteamCoreVerbose("SteamAPI_InitEx Result: %s", *ESteamAPIInitResultToString(SteamAPIInitResult));

		if (SteamAPIInitResult == ESteamAPIInitResult::k_ESteamAPIInitResult_OK)
		{
			UE_LOG(LogTemp, Verbose, TEXT("SteamAPI initialized"));
			m_bInitialized = true;
			return;
		}

		LogSteamCoreWarn("SteamAPI failed to initialize (%s)", UTF8_TO_TCHAR(ErrorMessage));
		return;
	}
#endif
	LogSteamCoreWarn("SteamAPI failed to initialize as the Dlls are not loaded.");
}

void FSteamCoreClientInstanceHandler::InternalShutdown()
{
	LogSteamCoreVerbose("Unloading the Steam API...");
#if WITH_STEAMCORE
	SteamAPI_Shutdown();
#endif
}


FSteamCoreServerInstanceHandler::FSteamCoreServerInstanceHandler(FSteamCoreSharedModule* SteamInitializer)
	: FSteamCoreInstanceHandlerBase()
{
	if (SteamInitializer == nullptr || !SteamInitializer->AreSteamDllsLoaded())
	{
		LogSteamCoreWarn("Steam Server API failed to initialize as the Dlls are not loaded.");
		return;
	}

	uint32 LocalServerIP = 0;
	FString MultiHome;
	if (FParse::Value(FCommandLine::Get(), TEXT("MULTIHOME="), MultiHome) && !MultiHome.IsEmpty())
	{
		const TSharedPtr<FInternetAddr> MultiHomeIP = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressFromString(MultiHome);
		if (MultiHomeIP.IsValid())
		{
			MultiHomeIP->GetIp(LocalServerIP);
		}
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("QueryPort="), m_QueryPort) == false)
	{
		if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("GameServerQueryPort"), m_QueryPort, GEngineIni))
		{
			m_QueryPort = 27015;
		}
	}

	bool bVACEnabled = false;
	GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bVACEnabled"), bVACEnabled, GEngineIni);

	FString GameVersion;
	GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameVersion"), GameVersion, GEngineIni);
	if (GameVersion.Len() == 0)
	{
		LogSteamCoreWarn("[OnlineSubsystemSteamCore].GameVersion is not set. Server advertising will fail");
	}

	LogSteamCoreVerbose("Initializing Steam Game Server IP: 0x%08X Port: %d QueryPort: %d", LocalServerIP, m_GamePort, m_QueryPort);

#if WITH_STEAMCORE
	SteamErrMsg ErrorMessage;
	const ESteamAPIInitResult SteamAPIInitResult = SteamGameServer_InitEx(LocalServerIP, m_GamePort, m_QueryPort, (bVACEnabled ? eServerModeAuthenticationAndSecure : eServerModeAuthentication), TCHAR_TO_UTF8(*GameVersion), &ErrorMessage);

	LogSteamCoreVerbose("SteamGameServer_InitEx Result: %s", *ESteamAPIInitResultToString(SteamAPIInitResult));
	if (SteamAPIInitResult == k_ESteamAPIInitResult_OK)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Steam Dedicated Server API initialized."));
		m_bInitialized = true;
	}
	else
	{
		LogSteamCoreWarn("Steam Dedicated Server API failed to initialize (%s)", UTF8_TO_TCHAR(ErrorMessage));
	}
#endif
}

void FSteamCoreServerInstanceHandler::InternalShutdown()
{
#if WITH_STEAMCORE
	if (SteamGameServer() && SteamGameServer()->BLoggedOn())
	{
		SteamGameServer()->LogOff();
	}

	LogSteamCoreVerbose("Unloading the Steam server API...");
	SteamGameServer_Shutdown();
#endif
}

bool FSteamCoreInstanceHandlerBase::CanCleanUp() const
{
	return m_bInitialized && FSteamCoreSharedModule::IsAvailable() && FSteamCoreSharedModule::Get().AreSteamDllsLoaded();
}

void FSteamCoreInstanceHandlerBase::Destroy()
{
	if (CanCleanUp())
	{
		InternalShutdown();
	}
}
