/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreNetConnection.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

USteamCoreNetConnection::USteamCoreNetConnection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  m_bIsPassthrough(false)
{
}

void USteamCoreNetConnection::InitLocalConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	m_bIsPassthrough = InURL.Host.StartsWith(STEAM_URL_PREFIX) ? false : true;
	if (!m_bIsPassthrough)
	{
		DisableAddressResolution();
	}

	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
	if (!m_bIsPassthrough && RemoteAddr.IsValid())
	{
		if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
		{
			SocketSubsystem->RegisterConnection(this);
		}
	}
#else
	Super::InitLocalConnection(InDriver, InSocket, InURL, InState, InMaxPacket, InPacketOverhead);
#endif
}

void USteamCoreNetConnection::InitRemoteConnection(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, const class FInternetAddr& InRemoteAddr, EConnectionState InState, int32 InMaxPacket, int32 InPacketOverhead)
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	m_bIsPassthrough = static_cast<USteamCoreNetDriver*>(InDriver)->m_bIsPassthrough;
	if (!m_bIsPassthrough)
	{
		DisableAddressResolution();
	}

	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
	if (!m_bIsPassthrough && RemoteAddr.IsValid())
	{
		if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
		{
			SocketSubsystem->RegisterConnection(this);
		}
	}
#else
	Super::InitRemoteConnection(InDriver, InSocket, InURL, InRemoteAddr, InState, InMaxPacket, InPacketOverhead);
#endif
}

void USteamCoreNetConnection::CleanUp()
{
	LogSteamCoreVerbose("");
	Super::CleanUp();

#if WITH_STEAMCORE
	if (!m_bIsPassthrough)
	{
		if (FSocketSubsystemSteamCore* SocketSubsystem = static_cast<FSocketSubsystemSteamCore*>(ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM)))
		{
			SocketSubsystem->UnregisterConnection(this);
		}
	}
#endif
}
