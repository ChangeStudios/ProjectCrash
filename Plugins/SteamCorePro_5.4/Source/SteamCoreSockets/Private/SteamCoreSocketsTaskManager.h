/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "SteamCoreSocketsTaskManagerInterface.h"
#include "SteamCoreSocketsPrivate.h"
#include "Containers/Queue.h"

#if WITH_STEAMCORE
class FSteamCoreSocketsTaskManager : public FSteamCoreSocketsTaskManagerInterface
{
public:
	FSteamCoreSocketsTaskManager(FSteamCoreSocketsSubsystem* SocketSub)
		: FSteamCoreSocketsTaskManagerInterface(SocketSub),
		  OnConnectionStatusChangedCallback(this, &FSteamCoreSocketsTaskManager::OnConnectionStatusChanged),
		  OnConnectionStatusChangedGSCallback(this, &FSteamCoreSocketsTaskManager::OnConnectionStatusChangedGS)
	{
	}

	virtual ~FSteamCoreSocketsTaskManager() override
	{
		m_MessageQueue.Empty();
	}

	virtual void Tick() override;


private:
	STEAM_CALLBACK(FSteamCoreSocketsTaskManager, OnConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t, OnConnectionStatusChangedCallback);
	STEAM_GAMESERVER_CALLBACK(FSteamCoreSocketsTaskManager, OnConnectionStatusChangedGS, SteamNetConnectionStatusChangedCallback_t, OnConnectionStatusChangedGSCallback);

	TQueue<SteamNetConnectionStatusChangedCallback_t> m_MessageQueue;
};
#endif
