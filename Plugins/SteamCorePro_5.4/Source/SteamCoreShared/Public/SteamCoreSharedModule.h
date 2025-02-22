/**
* Copyright (C) 2017-2024 eelDev AB
*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "SteamCoreSharedPackage.h"

#define LOADING_STEAM_CLIENT_LIBRARY_DYNAMICALLY		(PLATFORM_WINDOWS || PLATFORM_MAC || (PLATFORM_LINUX && !IS_MONOLITHIC))
#define LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY		(PLATFORM_WINDOWS || (PLATFORM_LINUX && !IS_MONOLITHIC) || PLATFORM_MAC)
#define LOADING_STEAM_LIBRARIES_DYNAMICALLY				(LOADING_STEAM_CLIENT_LIBRARY_DYNAMICALLY || LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY)

DECLARE_LOG_CATEGORY_EXTERN(LogSteamCoreShared, Log, All);

class STEAMCORESHARED_API FSteamCoreSharedModule : public IModuleInterface
{
public:
	FSteamCoreSharedModule()
		: m_SteamDLLHandle(nullptr),
		  m_SteamServerDLLHandle(nullptr),
		  m_bForceLoadSteamClientDll(false),
		  m_SteamClientObserver(nullptr),
		  m_SteamServerObserver(nullptr)
	{
	}

	virtual ~FSteamCoreSharedModule() override
	{
	}

	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	TSharedPtr<class FSteamCoreClientInstanceHandler> ObtainSteamClientInstanceHandle();
	TSharedPtr<class FSteamCoreServerInstanceHandler> ObtainSteamServerInstanceHandle();

	bool AreSteamDllsLoaded() const;
	FString GetSteamModulePath() const;
	bool IsLoadingServerClientDlls() const { return m_bForceLoadSteamClientDll; }
	bool CanLoadClientDllsOnServer() const;

	static inline class FSteamCoreSharedModule& Get()
	{
		return FModuleManager::LoadModuleChecked<class FSteamCoreSharedModule>("SteamCoreShared");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SteamCoreShared");
	}

private:
	void* m_SteamDLLHandle;
	void* m_SteamServerDLLHandle;
	bool m_bForceLoadSteamClientDll;

	TWeakPtr<FSteamCoreClientInstanceHandler> m_SteamClientObserver;
	TWeakPtr<FSteamCoreServerInstanceHandler> m_SteamServerObserver;

	void LoadSteamModules();
	void UnloadSteamModules();
};

class STEAMCORESHARED_API FSteamCoreInstanceHandlerBase
{
public:
	virtual ~FSteamCoreInstanceHandlerBase()
	{
	}

	virtual bool IsInitialized() const { return m_bInitialized; }
	int32 GetGamePort() const { return m_GamePort; }

protected:
	FSteamCoreInstanceHandlerBase();

	bool m_bInitialized;
	int32 m_GamePort;

	virtual bool CanCleanUp() const;
	virtual void Destroy();
	virtual void InternalShutdown() = 0;
};

class STEAMCORESHARED_API FSteamCoreClientInstanceHandler final : public FSteamCoreInstanceHandlerBase
{
public:
	virtual ~FSteamCoreClientInstanceHandler() override { Destroy(); }

PACKAGE_SCOPE:
	FSteamCoreClientInstanceHandler(FSteamCoreSharedModule* SteamInitializer);

protected:
	virtual void InternalShutdown() override;

private:
	FSteamCoreClientInstanceHandler()
		: FSteamCoreInstanceHandlerBase()
	{
	}
};

class STEAMCORESHARED_API FSteamCoreServerInstanceHandler final : public FSteamCoreInstanceHandlerBase
{
public:
	virtual ~FSteamCoreServerInstanceHandler() override { Destroy(); }

	int32 GetQueryPort() const { return m_QueryPort; }

PACKAGE_SCOPE:
	FSteamCoreServerInstanceHandler(FSteamCoreSharedModule* SteamInitializer);

protected:
	int32 m_QueryPort;
	virtual void InternalShutdown() override;

private:
	FSteamCoreServerInstanceHandler()
		: FSteamCoreInstanceHandlerBase(),
		  m_QueryPort(-1)
	{
	}
};
