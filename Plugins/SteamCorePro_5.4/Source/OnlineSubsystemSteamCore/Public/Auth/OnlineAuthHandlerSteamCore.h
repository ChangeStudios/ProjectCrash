/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCoreTypes.h"
#include "PacketHandler.h"
#include "HandlerComponentFactory.h"
#include "OnlineAuthHandlerTypesSteamCore.h"
#include "OnlineAuthHandlerSteamCore.generated.h"

#if WITH_STEAMCORE
class ONLINESUBSYSTEMSTEAMCORE_API FSteamCoreAuthHandlerComponent : public HandlerComponent
{
public:
	FSteamCoreAuthHandlerComponent();
	virtual ~FSteamCoreAuthHandlerComponent() override;
	virtual void CountBytes(FArchive& Ar) const override;
	virtual void Initialize() override;
	virtual void NotifyHandshakeBegin() override;

	virtual bool IsValid() const override;

	virtual void Incoming(FBitReader& Packet) override;
	virtual void Outgoing(FBitWriter& Packet, FOutPacketTraits& Traits) override;

	virtual void Tick(float DeltaTime) override;

	virtual int32 GetReservedPacketBits() const override;

protected:
	void SetState(ESteamAuthHandlerState NewState);
	void SetComponentReady();
	void SendAuthKey(bool bGenerateNewKey);
	void SendServerSteamId();
	bool SendAuthResult();
	void SendPacket(FBitWriter& OutboundPacket);
	void RequestResend();

protected:
	FOnlineAuthSteamCorePtr m_AuthInterface;
	ISteamUser* m_SteamUserPtr;
	ESteamAuthHandlerState m_State;
	bool m_bIsEnabled;
	float m_LastTimestamp;
	FString m_UserTicket;
	uint32 m_TicketHandle;
	FUniqueNetIdSteamRef m_SteamId;
	uint64 m_ServerSteamId;
};
#endif

UCLASS()
class USteamCoreAuthComponentModuleInterface : public UHandlerComponentFactory
{
	GENERATED_UCLASS_BODY()
public:
	virtual TSharedPtr<HandlerComponent> CreateComponentInstance(FString& Options) override;
};
