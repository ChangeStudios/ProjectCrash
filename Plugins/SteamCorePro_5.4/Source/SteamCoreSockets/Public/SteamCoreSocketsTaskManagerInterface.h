/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

class FSteamCoreSocketsTaskManagerInterface
{
public:
	FSteamCoreSocketsTaskManagerInterface(class FSteamCoreSocketsSubsystem* SocketSub) :
		m_SocketSubsystem(SocketSub)
	{
	}
	
	virtual ~FSteamCoreSocketsTaskManagerInterface() = default;
	virtual void Tick() = 0;

protected:
	FSteamCoreSocketsSubsystem* m_SocketSubsystem;
};
