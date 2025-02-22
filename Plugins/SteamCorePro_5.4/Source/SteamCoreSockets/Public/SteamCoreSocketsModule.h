/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSteamCoreSockets, Log, All);

class STEAMCORESOCKETS_API FSteamCoreSocketsModule : public IModuleInterface
{
public:
	FSteamCoreSocketsModule()
		: m_bEnabled(false)
	{
	}

	virtual ~FSteamCoreSocketsModule() override
	{
	}

	// IModuleInterface

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	static inline class FSteamCoreSocketsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<class FSteamCoreSocketsModule>("SteamCoreSockets");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SteamCoreSockets");
	}

	bool IsSteamCoreSocketsEnabled() const { return m_bEnabled; }

private:
	bool m_bEnabled;
};
