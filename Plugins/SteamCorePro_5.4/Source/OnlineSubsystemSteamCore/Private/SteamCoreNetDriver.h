/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "IpNetDriver.h"
#include "SteamCoreNetDriver.generated.h"

class Error;
class FNetworkNotify;

UCLASS(transient, config=Engine)
class USteamCoreNetDriver : public UIpNetDriver
{
	GENERATED_UCLASS_BODY()
	bool m_bIsPassthrough;

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface

	//~ Begin UIpNetDriver Interface

	virtual ISocketSubsystem* GetSocketSubsystem() override;
	virtual bool IsAvailable() const override;
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual void Shutdown() override;
	virtual bool IsNetResourceValid() override;

	//~ End UIpNetDriver Interface
};