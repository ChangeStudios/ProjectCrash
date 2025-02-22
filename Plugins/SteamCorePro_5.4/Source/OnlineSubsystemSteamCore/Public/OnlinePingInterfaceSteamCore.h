/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FOnlinePingInterfaceSteamCore
{
public:
	FOnlinePingInterfaceSteamCore(class FOnlineSubsystemSteamCore* InSubsystem)
		: m_Subsystem(InSubsystem)
	{
	}

	virtual ~FOnlinePingInterfaceSteamCore()
	{
	}

	virtual bool IsUsingP2PRelays() const = 0;
	virtual FString GetHostPingData() const = 0;
	virtual int32 GetPingFromHostData(const FString& HostPingStr) const = 0;
	virtual bool IsRecalculatingPing() const = 0;

protected:
	FOnlineSubsystemSteamCore* m_Subsystem;
};
#endif