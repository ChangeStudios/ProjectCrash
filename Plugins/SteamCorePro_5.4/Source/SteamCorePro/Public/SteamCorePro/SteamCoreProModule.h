/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SteamCorePro/SteamUtilities.h"
#include "SteamCoreProModule.generated.h"

class USteamCoreProMatchmakingServersAsyncActionRequestServerList;
class USteamCoreProMatchmakingServersAsyncActionPingServer;
class USteamCoreProMatchmakingServersAsyncActionServerRules;
class FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer;
class FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList;
class FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules;

DECLARE_LOG_CATEGORY_EXTERN(LogSteamCorePro, Log, All);

class STEAMCOREPRO_API FSteamCoreProModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
public:
	static FString s_PluginName;
	static FString s_PluginVersion;
};

UCLASS(abstract)
class STEAMCOREPRO_API USteamCorePro : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class STEAMCOREPRO_API USteamCoreInterface : public UObject
{
	GENERATED_BODY()
public:
	USteamCoreInterface() {};
	virtual ~USteamCoreInterface() override {};

	void QueueAsyncTask(class FOnlineAsyncTask* AsyncTask);
};

UCLASS()
class STEAMCOREPRO_API USteamCoreProSubsystem : public UObject
{
	GENERATED_BODY()

	friend class USteamCoreProMatchmakingServersAsyncActionRequestServerList;
	friend class USteamCoreProMatchmakingServersAsyncActionPingServer;
	friend class USteamCoreProMatchmakingServersAsyncActionServerRules;
	friend class FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer;
	friend class FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList;
	friend class FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules;
public:
	USteamCoreProSubsystem() {};
	virtual ~USteamCoreProSubsystem() override {};

	void QueueAsyncTask(FOnlineAsyncTask* AsyncTask);

	static USteamCoreProSubsystem* Get();

protected:
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerList* CurrentMatchmakingServersServerList; 
	FOnlineAsyncTaskSteamCoreProMatchmakingServersPingServer* CurrentMatchmakingServersPingServer; 
	FOnlineAsyncTaskSteamCoreProMatchmakingServersServerRules* CurrentMatchmakingServersServerRules; 
	
private:
	bool bInitialized = false;
};
