/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "OnlinePingInterfaceSteamCore.h"

#if WITH_STEAMCORE
class FSteamCoreSocketsPing : public FOnlinePingInterfaceSteamCore
{
public:
	FSteamCoreSocketsPing(class FSteamCoreSocketsSubsystem* InSocketSub, class FOnlineSubsystemSteamCore* InOnlineSub)
		: FOnlinePingInterfaceSteamCore(InOnlineSub),
		  m_SocketSub(InSocketSub)
	{
	}

	virtual bool IsUsingP2PRelays() const override;
	virtual FString GetHostPingData() const override;
	virtual int32 GetPingFromHostData(const FString& HostPingStr) const override;
	virtual bool IsRecalculatingPing() const override;

protected:
	FSteamCoreSocketsSubsystem* m_SocketSub;
};
#endif
