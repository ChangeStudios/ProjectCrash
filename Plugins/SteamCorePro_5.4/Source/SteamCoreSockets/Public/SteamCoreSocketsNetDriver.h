/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/NetDriver.h"
#include "SteamCoreSocketsPackage.h"
#include "SteamCoreSocketsTypes.h"
#include "SteamCoreSocketsNetDriver.generated.h"

class FNetworkNotify;

UCLASS(transient, config=Engine)
class STEAMCORESOCKETS_API USteamCoreSocketsNetDriver : public UNetDriver
{
	GENERATED_BODY()

public:
	USteamCoreSocketsNetDriver() :
		m_Socket(nullptr),
		m_bIsDelayedNetworkAccess(false)
	{
		RelevantTimeout = 5.0f;
		KeepAliveTime = 0.2f;
		NetServerMaxTickRate = 30;
		InitialConnectTimeout = 120.f;
		ConnectionTimeout = 80.f;
		MaxClientRate=1000000000;
		MaxInternetClientRate=1000000000;
		SpawnPrioritySeconds=1.0;
		ServerTravelPause=4.0;
		InitialConnectTimeout=120.0;
		NetConnectionClassName="/Script/SteamCoreSockets.SteamCoreSocketsNetConnection";
	}

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface

	//~ Begin UNetDriver Interface.
	virtual void Shutdown() override;
	virtual bool IsAvailable() const override;
	virtual bool InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error) override;
	virtual bool InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error) override;
	virtual bool InitListen(FNetworkNotify* InNotify, FURL& LocalURL, bool bReuseAddressAndPort, FString& Error) override;
	virtual void TickDispatch(float DeltaTime) override;
	virtual void LowLevelSend(TSharedPtr<const FInternetAddr> Address, void* Data, int32 CountBits, FOutPacketTraits& Traits) override;
	virtual void LowLevelDestroy() override;
	virtual class ISocketSubsystem* GetSocketSubsystem() override;
	virtual bool IsNetResourceValid(void) override;
	//~ End UNetDriver Interface

	bool ArePacketHandlersDisabled() const;

protected:
	class FSteamCoreSocket* m_Socket;
	bool m_bIsDelayedNetworkAccess;

	void ResetSocketInfo(const class FSteamCoreSocket* RemovedSocket);

	UNetConnection* FindClientConnectionForHandle(SteamCoreSocketHandles SocketHandle);

	void OnConnectionCreated(SteamCoreSocketHandles ListenParentHandle, SteamCoreSocketHandles SocketHandle);
	void OnConnectionUpdated(SteamCoreSocketHandles SocketHandle, int32 NewState);
	void OnConnectionDisconnected(SteamCoreSocketHandles SocketHandle);

	friend class FSteamCoreSocketsSubsystem;
};