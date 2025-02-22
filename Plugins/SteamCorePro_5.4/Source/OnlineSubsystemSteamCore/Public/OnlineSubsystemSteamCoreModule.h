/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOnlineSubsystemSteamCore, Log, All);

class FOnlineSubsystemSteamCoreModule : public IModuleInterface
{
public:
	FOnlineSubsystemSteamCoreModule()
        : m_SteamFactory(nullptr)
	{}

	virtual ~FOnlineSubsystemSteamCoreModule() override {}

	// IModuleInterface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}

private:
	class FOnlineFactorySteamCore* m_SteamFactory;
};
