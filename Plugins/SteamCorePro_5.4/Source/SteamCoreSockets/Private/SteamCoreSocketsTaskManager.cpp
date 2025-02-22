/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocketsTaskManager.h"
#include "SteamCoreSocketsSubsystem.h"

#if WITH_STEAMCORE
void FSteamCoreSocketsTaskManager::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* SteamConnectionMessage)
{
	m_MessageQueue.Enqueue(*SteamConnectionMessage);
}

void FSteamCoreSocketsTaskManager::OnConnectionStatusChangedGS(SteamNetConnectionStatusChangedCallback_t* SteamConnectionMessage)
{
	m_MessageQueue.Enqueue(*SteamConnectionMessage);
}

void FSteamCoreSocketsTaskManager::Tick()
{
	SteamNetConnectionStatusChangedCallback_t CallbackData;
	while (!m_MessageQueue.IsEmpty())
	{
		FMemory::Memzero(CallbackData);
		if (m_SocketSubsystem && m_MessageQueue.Dequeue(CallbackData))
		{
			m_SocketSubsystem->SteamCoreSocketEventHandler(&CallbackData);
		}
		else
		{
			break;
		}
	}
}
#endif
