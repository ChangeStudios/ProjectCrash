/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "SteamCoreSocketsSubsystem.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreMisc.h"
#include "Misc/ConfigCacheIni.h"
#include "IPAddressSteamCoreSockets.h"
#include "SteamCoreSocketsPrivate.h"
#include "SteamCoreSocketsTaskManager.h"
#include "SteamCoreSocketsNetDriver.h"
#include "SteamCoreSharedModule.h"
#include "SteamCoreSocket.h"
#include "SteamCoreSocketsPing.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteamCore.h"
#include "SteamCoreSocketsTypes.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogSteamCoreSocketsAPI, Log, All);

#if WITH_STEAMCORE

static FString ConnectionStateToString(const ESteamNetworkingConnectionState& ConnectionState)
{
	switch (ConnectionState)
	{
	default:
	case k_ESteamNetworkingConnectionState_None:
		return TEXT("None");
	case k_ESteamNetworkingConnectionState_Connecting:
		return TEXT("Connecting");
	case k_ESteamNetworkingConnectionState_FindingRoute:
		return TEXT("Finding Route");
	case k_ESteamNetworkingConnectionState_Connected:
		return TEXT("Connected");
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
		return TEXT("Closed by Peer");
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		return TEXT("Local Connection Issue");
	}
}

void SteamCoreSocketsDebugLogger(ESteamNetworkingSocketsDebugOutputType nType, const char* pszMsg)
{
#if !NO_LOGGING
	FString OutputType;
	ELogVerbosity::Type Verboseness = ELogVerbosity::Verbose;
	switch (nType)
	{
	default:
	case k_ESteamNetworkingSocketsDebugOutputType_None:
		OutputType = TEXT("None");
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Bug:
		OutputType = TEXT("Bug");
		Verboseness = ELogVerbosity::Error;
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Error:
		OutputType = TEXT("Error");
		Verboseness = ELogVerbosity::Error;
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Important:
		OutputType = TEXT("Important");
		Verboseness = ELogVerbosity::Warning;
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Warning:
		OutputType = TEXT("Warning");
		Verboseness = ELogVerbosity::Warning;
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Everything:
	case k_ESteamNetworkingSocketsDebugOutputType_Verbose:
	case k_ESteamNetworkingSocketsDebugOutputType_Msg:
		OutputType = TEXT("Log");
		break;
	case k_ESteamNetworkingSocketsDebugOutputType_Debug:
		OutputType = TEXT("Debug");
		break;
	}

	GLog->Log(LogSteamCoreSocketsAPI.GetCategoryName(), Verboseness, FString::Printf(TEXT("SteamCoreSockets API: %s %s"), *OutputType, ANSI_TO_TCHAR(pszMsg)));
#endif
}

FSteamCoreSocketsSubsystem* FSteamCoreSocketsSubsystem::m_SocketSingleton = nullptr;

FSteamCoreSocketsSubsystem* FSteamCoreSocketsSubsystem::Create()
{
	if (m_SocketSingleton == nullptr)
	{
		m_SocketSingleton = new FSteamCoreSocketsSubsystem();
	}

	return m_SocketSingleton;
}

void FSteamCoreSocketsSubsystem::Destroy()
{
	if (m_SocketSingleton != nullptr)
	{
		m_SocketSingleton->Shutdown();
		delete m_SocketSingleton;
		m_SocketSingleton = nullptr;
	}
}

bool FSteamCoreSocketsSubsystem::Init(FString& Error)
{
	const bool bIsDedicated = IsRunningDedicatedServer();

	if (bIsDedicated)
	{
		GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bAllowP2PPacketRelay"), m_bUseRelays, GEngineIni);
	}

#if !UE_BUILD_SHIPPING
	bool bOverrideRelays = false;
	if (FParse::Bool(FCommandLine::Get(), TEXT("SteamCoreSocketsRelays"), bOverrideRelays))
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Set relay setting to %d"), bOverrideRelays);
		m_bUseRelays = bOverrideRelays;
	}
#endif

	FOnlineSubsystemSteamCore* OnlineSteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (bIsDedicated)
	{
		m_SteamAPIServerHandle = FSteamCoreSharedModule::Get().ObtainSteamServerInstanceHandle();
		if (!m_SteamAPIServerHandle.IsValid())
		{
			Error = TEXT("SteamCoreSockets: Could not initialize the game server SteamAPI!");
			m_SteamAPIServerHandle.Reset();
			return false;
		}

		if (OnlineSteamSubsystem != nullptr)
		{
			m_SteamServerLoginDelegateHandle = OnlineSteamSubsystem->AddOnSteamServerLoginCompletedDelegate_Handle(FOnSteamServerLoginCompletedDelegate::CreateRaw(this, &FSteamCoreSocketsSubsystem::OnServerLoginComplete));
		}
		else
		{
			Error = TEXT("SteamCoreSockets: Cannot register dedicated server login listeners!");
			m_SteamAPIServerHandle.Reset();
			return false;
		}
	}
	else
	{
		m_SteamAPIClientHandle = FSteamCoreSharedModule::Get().ObtainSteamClientInstanceHandle();
		if (!m_SteamAPIClientHandle.IsValid())
		{
			Error = TEXT("SteamCoreSockets: could not obtain a handle to SteamAPI!");
			m_SteamAPIClientHandle.Reset();
			return false;
		}
	}

#if !PLATFORM_LINUX
	if (IsUsingRelayNetwork())
	{
		if (!SteamNetworkingUtils())
		{
			Error = TEXT("SteamCoreSockets: Cannot interface with SteamNetworkingUtils! Relay support relies on this functionality!");
			m_SteamAPIClientHandle.Reset();
			m_SteamAPIServerHandle.Reset();
			return false;
		}

		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Initializing Network Relay"));
		SteamNetworkingUtils()->InitRelayNetworkAccess();
	}

	if (GetSteamSocketsInterface())
	{
		GetSteamSocketsInterface()->InitAuthentication();
	}
#endif
	ESteamNetworkingSocketsDebugOutputType DebugLevel = k_ESteamNetworkingSocketsDebugOutputType_Msg;

#if UE_BUILD_SHIPPING
	DebugLevel = k_ESteamNetworkingSocketsDebugOutputType_Important;
#endif

	SteamNetworkingUtils()->SetDebugOutputFunction(DebugLevel, SteamCoreSocketsDebugLogger);

	m_SteamEventManager = MakeUnique<FSteamCoreSocketsTaskManager>(this);

	if (OnlineSteamSubsystem != nullptr && IsUsingRelayNetwork())
	{
		OnlineSteamSubsystem->SetPingInterface(MakeShared<FSteamCoreSocketsPing, ESPMode::ThreadSafe>(this, OnlineSteamSubsystem));
	}

	return true;
}

void FSteamCoreSocketsSubsystem::Shutdown()
{
	UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Cleaning up"));

	if (FOnlineSubsystemSteamCore* OnlineSteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM)))
	{
		OnlineSteamSubsystem->ClearOnSteamServerLoginCompletedDelegate_Handle(m_SteamServerLoginDelegateHandle);
		OnlineSteamSubsystem->SetPingInterface(nullptr);
	}

	if (m_SteamEventManager.IsValid())
	{
		m_SteamEventManager.Reset();
	}

	m_PendingListenerArray.Empty();
	m_SteamAPIClientHandle.Reset();
	m_SteamAPIServerHandle.Reset();
}

class FSocket* FSteamCoreSocketsSubsystem::CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType)
{
	FName ProtocolTypeToUse = ProtocolType;
	if (ProtocolTypeToUse.IsNone())
	{
		if (IsUsingRelayNetwork())
		{
			ProtocolTypeToUse = FNetworkProtocolTypes::SteamCoreSocketsP2P;
		}
		else
		{
			ProtocolTypeToUse = FNetworkProtocolTypes::SteamCoreSocketsIP;
		}
	}
	else if (ProtocolType != FNetworkProtocolTypes::SteamCoreSocketsP2P && ProtocolType != FNetworkProtocolTypes::SteamCoreSocketsIP)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Attempted to create a socket with a protocol outside of the steam network!"));
		return nullptr;
	}

	return static_cast<FSocket*>(new FSteamCoreSocket(SOCKTYPE_Streaming, SocketDescription, ProtocolTypeToUse));
}

void FSteamCoreSocketsSubsystem::DestroySocket(FSocket* Socket)
{
	if (Socket != nullptr)
	{
		const FSteamCoreSocket* SteamSocket = static_cast<FSteamCoreSocket*>(Socket);
		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Destroy socket called on %u"), SteamSocket->m_InternalHandle);

		FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(SteamSocket->m_InternalHandle);
		if (SocketInfo != nullptr)
		{
			if (USteamCoreSocketsNetDriver* SteamNetDriver = SocketInfo->m_NetDriver.Get())
			{
				SteamNetDriver->ResetSocketInfo(SteamSocket);
			}
			SocketInfo->m_Socket = nullptr;
			SocketInfo->MarkForDeletion();
		}

		Socket->Close();

		delete Socket;
	}
}

FAddressInfoResult FSteamCoreSocketsSubsystem::GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName, EAddressInfoFlags QueryFlags, const FName ProtocolTypeName, ESocketType SocketType)
{
	FAddressInfoResult ResultData(HostName, ServiceName);

	const bool bWildcardProtocol = ProtocolTypeName.IsNone();

	if ((HostName == nullptr && ServiceName == nullptr) || (ServiceName != nullptr && !FCString::IsNumeric(ServiceName)) || (ProtocolTypeName != FNetworkProtocolTypes::SteamCoreSocketsP2P && ProtocolTypeName != FNetworkProtocolTypes::SteamCoreSocketsIP && !bWildcardProtocol))
	{
		ResultData.ReturnCode = SE_EINVAL;
		return ResultData;
	}

	TArray<TSharedPtr<FInternetAddr>> AddressList;

	if (HostName == nullptr || EnumHasAnyFlags(QueryFlags, EAddressInfoFlags::BindableAddress))
	{
		GetLocalAdapterAddresses(AddressList);
	}
	else
	{
		const TSharedPtr<FInternetAddr> SerializedAddr = GetAddressFromString(HostName);
		if (SerializedAddr.IsValid())
		{
			AddressList.Add(SerializedAddr);
		}
	}

	if (AddressList.Num() >= 1)
	{
		ResultData.ReturnCode = SE_NO_ERROR;
		int32 PortToUse = -1;
		if (ServiceName != nullptr && FCString::IsNumeric(ServiceName))
		{
			PortToUse = FCString::Atoi(ServiceName);
		}

		for (auto& AddressItem : AddressList)
		{
			if (bWildcardProtocol || (!bWildcardProtocol && AddressItem->GetProtocolType() == ProtocolTypeName))
			{
				if (PortToUse >= 0)
				{
					AddressItem->SetPort(PortToUse);
				}
				ResultData.Results.Add(FAddressInfoResultData(AddressItem.ToSharedRef(), 0, AddressItem->GetProtocolType(), SOCKTYPE_Streaming));
			}
		}

		if (ResultData.Results.Num() <= 0)
		{
			if (!bWildcardProtocol)
			{
				ResultData.ReturnCode = SE_ADDRFAMILY;
			}
			else
			{
				ResultData.ReturnCode = SE_EFAULT;
			}
		}
	}
	else
	{
		ResultData.ReturnCode = SE_NO_DATA;
	}

	return ResultData;
}

TSharedPtr<FInternetAddr> FSteamCoreSocketsSubsystem::GetAddressFromString(const FString& IPAddress)
{
	TSharedRef<FInternetAddrSteamCoreSockets> NewAddr = StaticCastSharedRef<FInternetAddrSteamCoreSockets>(CreateInternetAddr());

	if (IPAddress.IsEmpty())
	{
		NewAddr->SetAnyAddress();

		return NewAddr;
	}

	bool bIsAddrValid = false;
	NewAddr->SetIp(*IPAddress, bIsAddrValid);
	if (!bIsAddrValid)
	{
		return nullptr;
	}

	return NewAddr;
}

bool FSteamCoreSocketsSubsystem::GetHostName(FString& HostName)
{
	UE_LOG(LogSockets, Warning, TEXT("GetHostName is not supported on SteamCoreSockets"));
	return false;
}

TSharedRef<FInternetAddr> FSteamCoreSocketsSubsystem::CreateInternetAddr()
{
	return MakeShareable(new FInternetAddrSteamCoreSockets());
}

TSharedRef<FInternetAddr> FSteamCoreSocketsSubsystem::CreateInternetAddr(const FName RequestedProtocol)
{
	return MakeShareable(new FInternetAddrSteamCoreSockets(RequestedProtocol));
}

const TCHAR* FSteamCoreSocketsSubsystem::GetSocketAPIName() const
{
	return TEXT("SteamCoreSockets");
}

bool FSteamCoreSocketsSubsystem::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses)
{
	if (IsUsingRelayNetwork())
	{
		const TSharedPtr<FInternetAddr> IdentityAddress = GetIdentityAddress();
		if (IdentityAddress.IsValid())
		{
			OutAddresses.Add(IdentityAddress);
		}
	}

	const TSharedRef<FInternetAddr> AnyAddress = CreateInternetAddr(FNetworkProtocolTypes::SteamCoreSocketsIP);
	AnyAddress->SetAnyAddress();
	OutAddresses.Add(AnyAddress);

	return true;
}

TArray<TSharedRef<FInternetAddr>> FSteamCoreSocketsSubsystem::GetLocalBindAddresses()
{
	TArray<TSharedRef<FInternetAddr>> OutAddresses;
	TArray<TSharedPtr<FInternetAddr>> AdapterAddresses;
	GetLocalAdapterAddresses(AdapterAddresses);

	TSharedRef<FInternetAddr> MultihomeAddress = CreateInternetAddr(FNetworkProtocolTypes::SteamCoreSocketsIP);
	if (GetMultihomeAddress(MultihomeAddress))
	{
		OutAddresses.Add(MultihomeAddress);
	}

	for (const auto& AdapterAddress : AdapterAddresses)
	{
		OutAddresses.Add(AdapterAddress.ToSharedRef());
	}

	return OutAddresses;
}

void FSteamCoreSocketsSubsystem::CleanSocketInformation(bool bForceClean)
{
	for (SocketHandleInfoMap::TIterator It(m_SocketInformationMap); It; ++It)
	{
		FSteamCoreSocketInformation& SocketInfo = It.Value();
		if (SocketInfo.IsMarkedForDeletion() || bForceClean)
		{
			USteamCoreSocketsNetDriver* NetDriverInfo = SocketInfo.m_NetDriver.Get();
			if (NetDriverInfo && SocketInfo.m_Parent == nullptr)
			{
				NetDriverInfo->Shutdown();
				SocketInfo.m_NetDriver.Reset();
			}

			if (SocketInfo.m_Socket)
			{
				DestroySocket(SocketInfo.m_Socket);
			}

			It.RemoveCurrent();
		}
	}
}

void FSteamCoreSocketsSubsystem::DumpSocketInformationMap() const
{
#if !UE_BUILD_SHIPPING
	if (m_SocketInformationMap.Num() < 1)
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Socket Information Map is empty!"));
		return;
	}

	UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Printing Socket Information Map:\n"));
	for (SocketHandleInfoMap::TConstIterator It(m_SocketInformationMap); It; ++It)
	{
		const FSteamCoreSocketInformation& SocketInfo = It.Value();
		UE_LOG(LogSockets, Log, TEXT("# %s"), *SocketInfo.ToString());
	}
#endif
}

bool FSteamCoreSocketsSubsystem::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FSteamCoreSocketsSubsystem_Tick);

	if (m_SteamEventManager.IsValid())
	{
		m_SteamEventManager->Tick();
	}

	CleanSocketInformation(false);
	return true;
}

bool FSteamCoreSocketsSubsystem::Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bIsHandled = false;

#if !UE_BUILD_SHIPPING
	if (FParse::Command(&Cmd, TEXT("PrintSteamSocketInfo")))
	{
		bIsHandled = true;
		DumpSocketInformationMap();
	}
	if (FParse::Command(&Cmd, TEXT("PrintPendingSteamSocketInfo")))
	{
		bIsHandled = true;
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Printing Pending Socket Info"));
		for (auto& Pending : m_PendingListenerArray)
		{
			UE_LOG(LogSockets, Log, TEXT("# %s"), *Pending.ToString());
		}
	}
	else if (FParse::Command(&Cmd, TEXT("ClearSteamSocketInfo")))
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Clearing all socket information!"));
		bIsHandled = true;
		CleanSocketInformation(true);
	}
	else if (FParse::Command(&Cmd, TEXT("TogglePeekMessaging")))
	{
		bIsHandled = true;
		m_bShouldTestPeek = !m_bShouldTestPeek;
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Set Peek Messaging to %d"), m_bShouldTestPeek);
	}
#endif

	return bIsHandled;
}

ISteamNetworkingSockets* FSteamCoreSocketsSubsystem::GetSteamSocketsInterface()
{
	return (SteamGameServerNetworkingSockets() != nullptr && IsRunningDedicatedServer()) ? SteamGameServerNetworkingSockets() : SteamNetworkingSockets();
}

bool FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation::operator==(const FInternetAddr& InAddr) const
{
	const FInternetAddrSteamCoreSockets SteamAddr = *((FInternetAddrSteamCoreSockets*)&InAddr);
	return *m_Addr == SteamAddr;
}

void FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation::MarkForDeletion()
{
	m_bMarkedForDeletion = true;
	const USteamCoreSocketsNetDriver* NetDriverObj = m_NetDriver.Get();
	if (m_Socket != nullptr && NetDriverObj)
	{
		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Handle %u marked for deletion. Has parent? %d NetDriver definition: %s"),
		       m_Socket->m_InternalHandle, (m_Parent != nullptr), *NetDriverObj->GetDescription());
	}
}

FString FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation::ToString() const
{
	return FString::Printf(TEXT("SocketInfo: Addr[%s], Socket[%u], Status[%d], Listener[%d], HasNetDriver[%d], MarkedForDeletion[%d]"), (m_Addr.IsValid() ? *m_Addr->ToString(true) : TEXT("INVALID")), (m_Socket != nullptr ? m_Socket->m_InternalHandle : 0), (m_Socket != nullptr ? (int32)m_Socket->GetConnectionState() : -1), (m_Parent != nullptr), m_NetDriver.IsValid(), m_bMarkedForDeletion);
}

FString FSteamCoreSocketsSubsystem::FSteamPendingSocketInformation::ToString() const
{
	const USteamCoreSocketsNetDriver* SteamNetDriver = NetDriver.Get();
	return FString::Printf(TEXT("PendingSocketInfo: NetDriver name %s"), (SteamNetDriver != nullptr) ? *SteamNetDriver->GetDescription() : TEXT("INVALID"));
}

FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation* FSteamCoreSocketsSubsystem::GetSocketInfo(SteamCoreSocketHandles InternalSocketHandle)
{
	if (InternalSocketHandle == k_HSteamNetConnection_Invalid)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Cannot get information on an invalid socket handle, returning null"));
		return nullptr;
	}

	return m_SocketInformationMap.Find(InternalSocketHandle);
}

FSteamCoreSocketsSubsystem::FSteamCoreSocketInformation* FSteamCoreSocketsSubsystem::GetSocketInfo(const FInternetAddr& ForAddress)
{
	if (!ForAddress.IsValid())
	{
		return nullptr;
	}

	for (SocketHandleInfoMap::TIterator It(m_SocketInformationMap); It; ++It)
	{
		if (It.Value() == ForAddress)
		{
			return &(It.Value());
		}
	}

	return nullptr;
}

void FSteamCoreSocketsSubsystem::AddSocket(const FInternetAddr& ForAddr, FSteamCoreSocket* NewSocket, FSteamCoreSocket* ParentSocket)
{
	if (NewSocket == nullptr || ForAddr.IsValid() == false)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Attempted to track invalid socket data!"));
		return;
	}

	STEAM_SDK_IGNORE_REDUNDANCY_START
	if (NewSocket->m_InternalHandle == k_HSteamListenSocket_Invalid || NewSocket->m_InternalHandle == k_HSteamNetConnection_Invalid)
	{
		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Dropped socket tracking for socket with invalid handle."));
		return;
	}
	STEAM_SDK_IGNORE_REDUNDANCY_END

	const FSteamCoreSocketInformation NewSocketInfo(ForAddr.Clone(), NewSocket, ParentSocket);
	if (m_SocketInformationMap.Find(NewSocket->m_InternalHandle) == nullptr)
	{
		UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Now tracking socket %u for addr %s, has parent? %d"), NewSocket->m_InternalHandle, *ForAddr.ToString(true), (ParentSocket != nullptr));
		m_SocketInformationMap.Add(NewSocket->m_InternalHandle, NewSocketInfo);
	}
	else
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: A socket with the address %s already exists, will not add another!"), *ForAddr.ToString(true));
	}
}

void FSteamCoreSocketsSubsystem::RemoveSocketsForListener(FSteamCoreSocket* ListenerSocket)
{
	if (ListenerSocket == nullptr)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Attempted to free sockets attached to an invalid listener!"));
		return;
	}

	UE_LOG(LogSockets, Log, TEXT("SteamCoreSockets: Closing all sockets attached to listener %u"), ListenerSocket->m_InternalHandle);
	for (SocketHandleInfoMap::TIterator It(m_SocketInformationMap); It; ++It)
	{
		FSteamCoreSocketInformation& SocketInfo = It.Value();
		if (SocketInfo.m_Parent == ListenerSocket)
		{
			UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Removed socket %u"), SocketInfo.m_Socket->m_InternalHandle);
			SocketInfo.m_Parent = nullptr;
			SocketInfo.m_NetDriver.Reset();
			SocketInfo.MarkForDeletion();
		}
	}
}

void FSteamCoreSocketsSubsystem::QueueRemoval(SteamCoreSocketHandles RemoveHandle)
{
	FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(RemoveHandle);
	if (SocketInfo != nullptr)
	{
		FString Address = (SocketInfo->m_Addr.IsValid()) ? SocketInfo->m_Addr->ToString(true) : TEXT("INVALID");
		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Marked socket %u with address %s for removal (pending)"), RemoveHandle, *Address);
		SocketInfo->MarkForDeletion();
	}
}

void FSteamCoreSocketsSubsystem::LinkNetDriver(FSocket* Socket, USteamCoreSocketsNetDriver* NewNetDriver)
{
	if (Socket == nullptr || NewNetDriver == nullptr)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Attempted to create an invalid socket/netdriver pairing!"));
		return;
	}

	const FSteamCoreSocket* SteamSocket = static_cast<FSteamCoreSocket*>(Socket);
	STEAM_SDK_IGNORE_REDUNDANCY_START
	if (SteamSocket->m_InternalHandle == k_HSteamListenSocket_Invalid || SteamSocket->m_InternalHandle == k_HSteamNetConnection_Invalid)
	{
		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Dropped netdriver link with socket that has invalid handle."));
		return;
	}
	STEAM_SDK_IGNORE_REDUNDANCY_END

	FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(SteamSocket->m_InternalHandle);
	if (SocketInfo != nullptr)
	{
		SocketInfo->m_NetDriver = NewNetDriver;
	}
}

void FSteamCoreSocketsSubsystem::AddDelayedListener(FSteamCoreSocket* ListenSocket, USteamCoreSocketsNetDriver* NewNetDriver)
{
	if (ListenSocket == nullptr || NewNetDriver == nullptr)
	{
		UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Add delayed listener cannot listen to invalid data!"));
		return;
	}
	FSteamPendingSocketInformation WatchedListener;
	WatchedListener.Socket = ListenSocket;
	WatchedListener.NetDriver = NewNetDriver;
	m_PendingListenerArray.Add(WatchedListener);
}

void FSteamCoreSocketsSubsystem::OnServerLoginComplete(bool bWasSuccessful)
{
	const TSharedPtr<FInternetAddrSteamCoreSockets> SteamIdentityAddr = StaticCastSharedPtr<FInternetAddrSteamCoreSockets>(GetIdentityAddress());
	for (auto& DelayedListener : m_PendingListenerArray)
	{
		USteamCoreSocketsNetDriver* SteamNetDriver = DelayedListener.NetDriver.Get();
		if (SteamNetDriver == nullptr)
		{
			continue;
		}

		if (!bWasSuccessful)
		{
			SteamNetDriver->Shutdown();
			continue;
		}

		if (SteamIdentityAddr.IsValid())
		{
			const int32 ListenPort = DelayedListener.Socket->m_BindAddress.GetPlatformPort();
			TSharedPtr<FInternetAddrSteamCoreSockets> ListenerAddress = StaticCastSharedRef<FInternetAddrSteamCoreSockets>(SteamIdentityAddr->Clone());
			ListenerAddress->SetPlatformPort(ListenPort);

			DelayedListener.Socket->m_BindAddress = *ListenerAddress;
			SteamNetDriver->LocalAddr = ListenerAddress;

			if (!DelayedListener.Socket->Listen(0))
			{
				SteamNetDriver->Shutdown();
				continue;
			}
		}

		LinkNetDriver(DelayedListener.Socket, SteamNetDriver);

		SteamNetDriver->m_bIsDelayedNetworkAccess = false;

		UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Processed delayed listener: %s"), *DelayedListener.ToString());
	}

	m_PendingListenerArray.Empty();
}

TSharedPtr<FInternetAddr> FSteamCoreSocketsSubsystem::GetIdentityAddress()
{
	SteamNetworkingIdentity SteamIDData;
	TSharedRef<FInternetAddrSteamCoreSockets> SteamAddr = StaticCastSharedRef<FInternetAddrSteamCoreSockets>(CreateInternetAddr(FNetworkProtocolTypes::SteamCoreSocketsP2P));

	if (GetSteamSocketsInterface() && GetSteamSocketsInterface()->GetIdentity(&SteamIDData))
	{
		SteamAddr->m_Addr = SteamIDData;
		return SteamAddr;
	}
	else if (!IsRunningDedicatedServer() && SteamUser())
	{
		const CSteamID CurrentUser = SteamUser()->GetSteamID();
		if (CurrentUser.IsValid())
		{
			SteamAddr->m_Addr.SetSteamID(CurrentUser);
		}
		else
		{
			UE_LOG(LogSockets, Warning, TEXT("SteamCoreSockets: Unable to process current user's steam id!"));
		}
	}

	return nullptr;
}

bool FSteamCoreSocketsSubsystem::IsLoggedInToSteam() const
{
	return ((SteamGameServer() && IsRunningDedicatedServer()) ? SteamGameServer()->BLoggedOn() : ((SteamUser()) ? SteamUser()->BLoggedOn() : false));
}

void FSteamCoreSocketsSubsystem::SteamCoreSocketEventHandler(struct SteamNetConnectionStatusChangedCallback_t* Message)
{
	if (Message == nullptr || FSteamCoreSocketsSubsystem::GetSteamSocketsInterface() == nullptr)
	{
		return;
	}

	if (Message->m_eOldState == k_ESteamNetworkingConnectionState_None &&
		Message->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting &&
		Message->m_info.m_hListenSocket != k_HSteamListenSocket_Invalid)
	{
		const FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(Message->m_info.m_hListenSocket);
		USteamCoreSocketsNetDriver* NetDriver = SocketInfo->m_NetDriver.Get();
		if (SocketInfo && NetDriver)
		{
			NetDriver->OnConnectionCreated(Message->m_info.m_hListenSocket, Message->m_hConn);
		}
	}
	else if ((Message->m_eOldState == k_ESteamNetworkingConnectionState_Connecting || Message->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute) &&
		Message->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
	{
		const FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(Message->m_hConn);
		USteamCoreSocketsNetDriver* NetDriver = SocketInfo->m_NetDriver.Get();
		if (SocketInfo && NetDriver)
		{
			NetDriver->OnConnectionUpdated(Message->m_hConn, (int32)Message->m_info.m_eState);
		}
	}
	else if ((Message->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute || Message->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
			|| Message->m_eOldState == k_ESteamNetworkingConnectionState_Connected) &&
		(Message->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer || Message->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally))
	{
		FSteamCoreSocketInformation* SocketInfo = GetSocketInfo(Message->m_hConn);

		if (SocketInfo != nullptr && SocketInfo->m_Socket != nullptr && !SocketInfo->m_Socket->m_bIsListenSocket)
		{
			UE_LOG(LogSockets, Verbose, TEXT("SteamCoreSockets: Connection %u has disconnected. Old state: %s Reason: %s"), Message->m_hConn, *ConnectionStateToString(Message->m_eOldState), *ConnectionStateToString(Message->m_info.m_eState));

			if (USteamCoreSocketsNetDriver* NetDriver = SocketInfo->m_NetDriver.Get())
			{
				NetDriver->OnConnectionDisconnected(Message->m_hConn);
			}

			if (SocketInfo->m_Parent != nullptr || Message->m_info.m_hListenSocket == k_HSteamListenSocket_Invalid)
			{
				SocketInfo->MarkForDeletion();
			}
		}
	}
}
#endif