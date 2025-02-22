/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#pragma once

#include "CoreMinimal.h"
#include "Misc/CoreMisc.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Containers/Ticker.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "SocketTypes.h"
#include "SteamCoreSocketsTypes.h"
#include "SteamCoreSocketsTaskManagerInterface.h"
#include "SteamCoreSocketsNetDriver.h"
#include "Misc/EngineVersionComparison.h"

class FSteamCoreSocket;
class USteamCoreSocketsNetDriver;

#if UE_VERSION_OLDER_THAN(5,0,0)
typedef FTickerObjectBase FTSTickerObjectBase;
#endif

#if WITH_STEAMCORE
class STEAMCORESOCKETS_API FSteamCoreSocketsSubsystem : public ISocketSubsystem, public FTSTickerObjectBase, public FSelfRegisteringExec
{
public:
	FSteamCoreSocketsSubsystem()
		: m_bShouldTestPeek(false),
		  m_LastSocketError(0),
		  m_bUseRelays(true),
		  m_SteamEventManager(nullptr),
		  m_SteamAPIClientHandle(nullptr),
		  m_SteamAPIServerHandle(nullptr)
	{
	}

	//~ Begin SocketSubsystem Interface
	virtual bool Init(FString& Error) override;
	virtual void Shutdown() override;

	virtual FSocket* CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType) override;
	virtual void DestroySocket(class FSocket* Socket) override;

	virtual FAddressInfoResult GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName = nullptr, EAddressInfoFlags QueryFlags = EAddressInfoFlags::Default, const FName ProtocolTypeName = NAME_None, ESocketType SocketType = ESocketType::SOCKTYPE_Unknown) override;
	virtual TSharedPtr<FInternetAddr> GetAddressFromString(const FString& IPAddress) override;

	virtual bool GetHostName(FString& HostName) override;

	virtual TSharedRef<FInternetAddr> CreateInternetAddr() override;
	virtual TSharedRef<FInternetAddr> CreateInternetAddr(const FName RequestedProtocol) override;

	virtual const TCHAR* GetSocketAPIName() const override;

	virtual ESocketErrors GetLastErrorCode() override { return (ESocketErrors)m_LastSocketError; }
	virtual ESocketErrors TranslateErrorCode(int32 Code) override { return (ESocketErrors)Code; }

	virtual bool GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses) override;
	virtual TArray<TSharedRef<FInternetAddr>> GetLocalBindAddresses() override;
	virtual bool HasNetworkDevice() override { return true; }
	virtual bool IsSocketWaitSupported() const override { return false; }
	virtual bool RequiresChatDataBeSeparate() override { return false; }
	virtual bool RequiresEncryptedPackets() override { return false; }
	//~ End SocketSubsystem Interface

	//~ Begin FTickerObject Interface
	virtual bool Tick(float DeltaTime) override;
	//~ End FTickerObject Interface

	//~ Begin FSelfRegisteringExec Interface
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	//~ End FSelfRegisteringExec Interface

	bool IsUsingRelayNetwork() const { return m_bUseRelays; }
	bool IsSteamInitialized() const { return m_SteamAPIClientHandle.IsValid() || m_SteamAPIServerHandle.IsValid(); }

	static class ISteamNetworkingSockets* GetSteamSocketsInterface();

PACKAGE_SCOPE:
	struct FSteamCoreSocketInformation
	{
		FSteamCoreSocketInformation(TSharedPtr<FInternetAddr> InAddr, FSteamCoreSocket* InSocket, FSteamCoreSocket* InParent = nullptr)
			: m_Addr(InAddr),
			  m_Socket(InSocket),
			  m_Parent(InParent),
			  m_NetDriver(nullptr),
			  m_bMarkedForDeletion(false)
		{
		}

		void MarkForDeletion();

		bool IsMarkedForDeletion() const { return m_bMarkedForDeletion; }

		bool operator==(const FSteamCoreSocket* RHS) const
		{
			return m_Socket == RHS;
		}

		bool operator==(const FInternetAddr& InAddr) const;

		bool operator==(const FSteamCoreSocketInformation& RHS) const
		{
			return RHS.m_Addr == m_Addr && RHS.m_Socket == m_Socket;
		}

		bool IsValid() const
		{
			return m_Addr.IsValid() && m_Parent != nullptr;
		}

		FString ToString() const;

		TSharedPtr<FInternetAddr> m_Addr;
		FSteamCoreSocket* m_Socket;
		FSteamCoreSocket* m_Parent;
		TWeakObjectPtr<USteamCoreSocketsNetDriver> m_NetDriver;
	private:
		bool m_bMarkedForDeletion;
	};

	FSteamCoreSocketInformation* GetSocketInfo(SteamCoreSocketHandles InternalSocketHandle);
	FSteamCoreSocketInformation* GetSocketInfo(const FInternetAddr& ForAddress);

	void AddSocket(const FInternetAddr& ForAddr, FSteamCoreSocket* NewSocket, FSteamCoreSocket* ParentSocket = nullptr);
	void RemoveSocketsForListener(FSteamCoreSocket* ListenerSocket);
	void QueueRemoval(SteamCoreSocketHandles SocketHandle);
	void LinkNetDriver(FSocket* Socket, USteamCoreSocketsNetDriver* NewNetDriver);
	void AddDelayedListener(FSteamCoreSocket* ListenSocket, USteamCoreSocketsNetDriver* NewNetDriver);
	void OnServerLoginComplete(bool bWasSuccessful);

	TSharedPtr<FInternetAddr> GetIdentityAddress();
	bool IsLoggedInToSteam() const;

	static FSteamCoreSocketsSubsystem* Create();
	static void Destroy();
	void SteamCoreSocketEventHandler(struct SteamNetConnectionStatusChangedCallback_t* ConnectionEvent);

	bool m_bShouldTestPeek;
	int32 m_LastSocketError;

protected:
	struct FSteamPendingSocketInformation
	{
		FSteamCoreSocket* Socket;
		TWeakObjectPtr<USteamCoreSocketsNetDriver> NetDriver;

		FString ToString() const;
	};

	void CleanSocketInformation(bool bForceClean);
	void DumpSocketInformationMap() const;

	bool m_bUseRelays;
	static FSteamCoreSocketsSubsystem* m_SocketSingleton;
	TUniquePtr<class FSteamCoreSocketsTaskManagerInterface> m_SteamEventManager;
	TSharedPtr<class FSteamCoreClientInstanceHandler> m_SteamAPIClientHandle;
	TSharedPtr<class FSteamCoreServerInstanceHandler> m_SteamAPIServerHandle;
	typedef TMap<SteamCoreSocketHandles, FSteamCoreSocketInformation> SocketHandleInfoMap;
	SocketHandleInfoMap m_SocketInformationMap;
	TArray<FSteamPendingSocketInformation> m_PendingListenerArray;
	FDelegateHandle m_SteamServerLoginDelegateHandle;
};
#endif
