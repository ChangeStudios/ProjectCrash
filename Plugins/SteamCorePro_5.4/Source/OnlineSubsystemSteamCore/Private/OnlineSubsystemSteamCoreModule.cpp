/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "OnlineSubsystemSteamCoreModule.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"
#include "Misc/MessageDialog.h"


IMPLEMENT_MODULE(FOnlineSubsystemSteamCoreModule, OnlineSubsystemSteamCore);

DEFINE_LOG_CATEGORY(LogOnlineSubsystemSteamCore);

void* HackDeleteFunctionPointerSteam = (void*)(void(*)(void*))(::operator delete[]);

class FOnlineFactorySteamCore final : public IOnlineFactory
{
	void DestroySubsystem()
	{
		#if WITH_STEAMCORE
		if (s_SteamSingleton.IsValid())
		{
			s_SteamSingleton->Shutdown();
			s_SteamSingleton = nullptr;
		}
        #endif
	}

public:
	FOnlineFactorySteamCore()
	{
	}

	virtual ~FOnlineFactorySteamCore() override
	{
		DestroySubsystem();
	}

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName) override
	{
#if WITH_STEAMCORE
		if (!s_SteamSingleton.IsValid())
		{
			s_SteamSingleton = MakeShared<FOnlineSubsystemSteamCore, ESPMode::ThreadSafe>(InstanceName);
			if (s_SteamSingleton->IsEnabled())
			{
				if (!s_SteamSingleton->Init())
				{
					LogSteamCoreWarn("Steam API failed to initialize!");
					DestroySubsystem();
				}
			}
			else
			{
				UE_CLOG_ONLINE(IsRunningDedicatedServer() || IsRunningGame(), Warning, TEXT("Steam API disabled!"));
				DestroySubsystem();
			}

			return s_SteamSingleton;
		}

		LogSteamCoreWarn("Can't create more than one instance of Steam online subsystem!");
		return nullptr;
#else
		LogSteamCoreWarn("SteamCore is disabled -- skip creating subsystem.");
		return nullptr;
#endif
	}

private:
#if WITH_STEAMCORE
	static FOnlineSubsystemSteamCorePtr s_SteamSingleton;
#endif
};

#if WITH_STEAMCORE
FOnlineSubsystemSteamCorePtr FOnlineFactorySteamCore::s_SteamSingleton = nullptr;
#endif

void FOnlineSubsystemSteamCoreModule::StartupModule()
{
	FSteamCoreSharedModule& SharedModule = FSteamCoreSharedModule::Get();

	if (SharedModule.AreSteamDllsLoaded())
	{
		m_SteamFactory = new FOnlineFactorySteamCore();

		FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
		OSS.RegisterPlatformService(STEAMCORE_SUBSYSTEM, m_SteamFactory);
	}
	else
	{
		LogSteamCoreWarn("Steam SDK %s libraries not present at or failed to load!", *SharedModule.GetSteamModulePath());
	}
}

void FOnlineSubsystemSteamCoreModule::ShutdownModule()
{
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService(STEAMCORE_SUBSYSTEM);

	delete m_SteamFactory;
	m_SteamFactory = nullptr;
}
