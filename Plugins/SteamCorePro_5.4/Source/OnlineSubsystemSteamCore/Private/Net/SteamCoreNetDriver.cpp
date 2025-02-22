/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

#include "SteamCoreNetDriver.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 
#include "Misc/CommandLine.h"

USteamCoreNetDriver::USteamCoreNetDriver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  m_bIsPassthrough(false)
{
	RelevantTimeout = 5.0f;
	KeepAliveTime = 0.2f;
#if UE_VERSION_NEWER_THAN(5,3,2)
	SetNetServerMaxTickRate(30);
#else
	NetServerMaxTickRate = 30;
#endif
	InitialConnectTimeout = 120.f;
	ConnectionTimeout = 80.f;
	MaxClientRate = 1000000000;
	MaxInternetClientRate = 1000000000;
	InitialConnectTimeout = 120.0;
	SpawnPrioritySeconds = 1.0;
	ServerTravelPause = 4.0;
	MaxPortCountToTry = 100;
	NetConnectionClassName = "/Script/OnlineSubsystemSteamCore.SteamCoreNetConnection";
}

void USteamCoreNetDriver::PostInitProperties()
{
	Super::PostInitProperties();
}

bool USteamCoreNetDriver::IsAvailable() const
{
	LogSteamCoreVeryVerbose("");
	if (IOnlineSubsystem* SteamSubsystem = IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM))
	{
		if (ISocketSubsystem* SteamSockets = ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM))
		{
			return true;
		}
	}

	return false;
}

ISocketSubsystem* USteamCoreNetDriver::GetSocketSubsystem()
{
	LogSteamCoreVeryVerbose("");
	return ISocketSubsystem::Get(m_bIsPassthrough ? PLATFORM_SOCKETSUBSYSTEM : STEAMCORE_SUBSYSTEM);
}

bool USteamCoreNetDriver::InitBase(bool bInitAsClient, FNetworkNotify* InNotify, const FURL& URL, bool bReuseAddressAndPort, FString& Error)
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	if (m_bIsPassthrough)
	{
		return UIpNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
	}

	if (!UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error))
	{
		return false;
	}

	ISocketSubsystem* SocketSubsystem = GetSocketSubsystem();
	if (SocketSubsystem == nullptr)
	{
		UE_LOG(LogNet, Warning, TEXT("Unable to find socket subsystem"));
		Error = TEXT("Unable to find socket subsystem");
		return false;
	}

	if (GetSocket() == nullptr)
	{
		Error = FString::Printf(TEXT("SteamSockets: socket failed (%i)"), (int32)SocketSubsystem->GetLastErrorCode());
		return false;
	}

	LocalAddr = SocketSubsystem->GetLocalBindAddr(*GLog);
	LocalAddr->SetPort(URL.Port);

	int32 AttemptPort = LocalAddr->GetPort();
	const int32 BoundPort = SocketSubsystem->BindNextPort(GetSocket(), *LocalAddr, MaxPortCountToTry + 1, 1);
	UE_LOG(LogNet, Display, TEXT("%s bound to port %d"), *GetName(), BoundPort);
	return true;
#endif
	return false;
}

bool USteamCoreNetDriver::InitConnect(FNetworkNotify* InNotify, const FURL& ConnectURL, FString& Error)
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	if (ISocketSubsystem* SteamSockets = ISocketSubsystem::Get(STEAMCORE_SUBSYSTEM))
	{
		if (ConnectURL.Host.StartsWith(STEAM_URL_PREFIX))
		{
			FUniqueSocket NewSocket = SteamSockets->CreateUniqueSocket(FName(TEXT("SteamClientSocket")), TEXT("Unreal client (Steam)"), FNetworkProtocolTypes::SteamCore);

			TSharedPtr<FSocket> SharedSocket(NewSocket.Release(), FSocketDeleter(NewSocket.GetDeleter()));

			SetSocketAndLocalAddress(SharedSocket);
		}
		else
		{
			m_bIsPassthrough = true;
		}
	}
#endif

	return Super::InitConnect(InNotify, ConnectURL, Error);
}

bool USteamCoreNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	ISocketSubsystem* SteamSockets = ISocketSubsystem::Get(STEAM_SUBSYSTEM);
	if (SteamSockets && !ListenURL.HasOption(TEXT("bIsLanMatch")) && !FParse::Param(FCommandLine::Get(), TEXT("forcepassthrough")))
	{
		FName SocketTypeName = IsRunningDedicatedServer() ? FName(TEXT("SteamServerSocket")) : FName(TEXT("SteamClientSocket"));
		FUniqueSocket NewSocket = SteamSockets->CreateUniqueSocket(SocketTypeName, TEXT("Unreal server (Steam)"), FNetworkProtocolTypes::SteamCore);
		TSharedPtr<FSocket> SharedSocket(NewSocket.Release(), FSocketDeleter(NewSocket.GetDeleter()));

		SetSocketAndLocalAddress(SharedSocket);
	}
	else
	{
		m_bIsPassthrough = true;
	}
#endif
	return Super::InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void USteamCoreNetDriver::Shutdown()
{
#if WITH_STEAMCORE
	LogSteamCoreVerbose("");
	if (!m_bIsPassthrough)
	{
		if (FSocketSteamCore* SteamSocket = static_cast<FSocketSteamCore*>(GetSocket()))
		{
			SteamSocket->SetSteamSendMode(k_EP2PSendUnreliableNoDelay);
		}
	}
#endif

	Super::Shutdown();
}

bool USteamCoreNetDriver::IsNetResourceValid()
{
#if WITH_STEAMCORE
	LogSteamCoreVeryVerbose("");
	const bool bIsValidSteamSocket = !m_bIsPassthrough && (GetSocket() != nullptr) && static_cast<FSocketSteamCore*>(GetSocket())->m_LocalSteamId->IsValid();
	const bool bIsValidPassthroughSocket = m_bIsPassthrough && UIpNetDriver::IsNetResourceValid();
	return bIsValidSteamSocket || bIsValidPassthroughSocket;
#endif
	return false;
}
