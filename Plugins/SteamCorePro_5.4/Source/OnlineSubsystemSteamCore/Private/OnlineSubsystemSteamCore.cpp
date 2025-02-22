/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "OnlineSubsystemSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 
#include "Voice/VoiceEngineSteamCore.h"
#include "SharedCloud/OnlineCloudTypesSteamCore.h"

extern "C" {
static void __cdecl SteamworksWarningMessageHook(int Severity, const char* Message);
static void __cdecl SteamworksWarningMessageHookNoOp(int Severity, const char* Message);
}

static void __cdecl SteamworksWarningMessageHook(int Severity, const char* Message)
{
	const TCHAR* MessageType;
	switch (Severity)
	{
	case 0: MessageType = TEXT("message");
		break;
	case 1: MessageType = TEXT("warning");
		break;
	default: MessageType = TEXT("notification");
		break;
	}
	LogSteamCoreWarn("Steamworks SDK %s: %s", MessageType, UTF8_TO_TCHAR(Message));
}

static void __cdecl SteamworksWarningMessageHookNoOp(int Severity, const char* Message)
{
}

class FScopeSandboxContext
{
private:
	bool bSandboxWasEnabled;

	FScopeSandboxContext()
		: bSandboxWasEnabled(false)
	{
	}

public:
	FScopeSandboxContext(bool bSandboxEnabled)
	{
		bSandboxWasEnabled = IFileManager::Get().IsSandboxEnabled();
		IFileManager::Get().SetSandboxEnabled(bSandboxEnabled);
	}

	~FScopeSandboxContext()
	{
		IFileManager::Get().SetSandboxEnabled(bSandboxWasEnabled);
	}
};

inline FString GetSteamAppIdFilename()
{
	return FString::Printf(TEXT("%s%s"), FPlatformProcess::BaseDir(), STEAMAPPIDFILENAME);
}

#if !UE_BUILD_SHIPPING && !UE_BUILD_SHIPPING_WITH_EDITOR
static bool WriteSteamAppIdToDisk(int32 SteamAppId)
{
	if (SteamAppId > 0)
	{
		FScopeSandboxContext ScopedSandbox(false);
		const FString SteamAppIdFilename = GetSteamAppIdFilename();
		IFileHandle* Handle = IPlatformFile::GetPlatformPhysical().OpenWrite(*SteamAppIdFilename, false, false);
		if (!Handle)
		{
			LogSteamCoreError("Failed to create file: %s", *SteamAppIdFilename);
			return false;
		}
		else
		{
			const FString AppId = FString::Printf(TEXT("%d"), SteamAppId);

			FBufferArchive Archive;
			Archive.Serialize((void*)TCHAR_TO_ANSI(*AppId), AppId.Len());

			Handle->Write(Archive.GetData(), Archive.Num());
			delete Handle;
			Handle = nullptr;

			return true;
		}
	}

	LogSteamCoreWarn("Steam App Id provided (%d) is invalid, must be greater than 0!", SteamAppId);
	return false;
}

static void DeleteSteamAppIdFromDisk()
{
	const FString SteamAppIdFilename = GetSteamAppIdFilename();
	FScopeSandboxContext ScopedSandbox(false);
	if (FPaths::FileExists(SteamAppIdFilename))
	{
		bool bSuccessfullyDeleted = IFileManager::Get().Delete(*SteamAppIdFilename);
	}
}

#endif // !UE_BUILD_SHIPPING && !UE_BUILD_SHIPPING_WITH_EDITOR

bool ConfigureSteamCoreInitDevOptions(bool& RequireRelaunch, int32& RelaunchAppId)
{
#if !UE_BUILD_SHIPPING && !UE_BUILD_SHIPPING_WITH_EDITOR
	if (!GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamDevAppId"), RelaunchAppId, GEngineIni))
	{
		LogSteamCoreError("Missing SteamDevAppId key in OnlineSubsystemSteamCore of DefaultEngine.ini")
		return false;
	}
	else
	{
		if (!WriteSteamAppIdToDisk(RelaunchAppId))
		{
			LogSteamCoreError("Could not create/update the steam_appid.txt file! Make sure the directory is writable and there isn't another instance using this file");
			return false;
		}
	}

	GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bRelaunchInSteam"), RequireRelaunch, GEngineIni);

#else
	RelaunchAppId = 0;
	GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamAppId"), RelaunchAppId, GEngineIni);
	RequireRelaunch = true;
#endif

	return true;
}

#if WITH_STEAMCORE
FOnlineAuthSteamCorePtr FOnlineSubsystemSteamCore::GetAuthInterface() const
{
	return m_AuthInterface;
}

FOnlineAuthSteamCoreUtilsPtr FOnlineSubsystemSteamCore::GetAuthInterfaceUtils() const
{
	return m_AuthInterfaceUtils;
}

FOnlinePingSteamCorePtr FOnlineSubsystemSteamCore::GetPingInterface() const
{
	return m_PingInterface;
}

void FOnlineSubsystemSteamCore::SetPingInterface(FOnlinePingSteamCorePtr InPingInterface)
{
	m_PingInterface = InPingInterface;
}

FOnlineEncryptedAppTicketSteamCorePtr FOnlineSubsystemSteamCore::GetEncryptedAppTicketInterface() const
{

	return m_EncryptedAppTicketInterface;
}

IOnlineSessionPtr FOnlineSubsystemSteamCore::GetSessionInterface() const
{
	return m_SessionInterface;
}

IOnlineFriendsPtr FOnlineSubsystemSteamCore::GetFriendsInterface() const
{
	return m_FriendInterface;
}

IOnlineGroupsPtr FOnlineSubsystemSteamCore::GetGroupsInterface() const
{
	return nullptr;
}

IOnlinePartyPtr FOnlineSubsystemSteamCore::GetPartyInterface() const
{
	return nullptr;
}

IOnlineSharedCloudPtr FOnlineSubsystemSteamCore::GetSharedCloudInterface() const
{
	return m_SharedCloudInterface;
}

IOnlineUserCloudPtr FOnlineSubsystemSteamCore::GetUserCloudInterface() const
{
	return m_UserCloudInterface;
}

IOnlineLeaderboardsPtr FOnlineSubsystemSteamCore::GetLeaderboardsInterface() const
{
	return m_LeaderboardsInterface;
}

IOnlineVoicePtr FOnlineSubsystemSteamCore::GetVoiceInterface() const
{
	LogSteamCoreVeryVerbose("m_VoiceInterface IsValid: %d, m_bVoiceInterfaceInitialized: %d", m_VoiceInterface.IsValid(), m_bVoiceInterfaceInitialized);
	
	if (m_VoiceInterface.IsValid() && !m_bVoiceInterfaceInitialized)
	{
		if (!m_VoiceInterface->Init())
		{
			LogSteamCoreVerbose("Failed to init voice interface");
			
			m_VoiceInterface = nullptr;
			m_bVoiceInterfaceInitialized = false;
		}
		
		m_bVoiceInterfaceInitialized = true;
	}

	return m_VoiceInterface;
}

IOnlineExternalUIPtr FOnlineSubsystemSteamCore::GetExternalUIInterface() const
{
	return m_ExternalUIInterface;
}

IOnlineTimePtr FOnlineSubsystemSteamCore::GetTimeInterface() const
{
	return nullptr;
}

IOnlineIdentityPtr FOnlineSubsystemSteamCore::GetIdentityInterface() const
{
	return m_IdentityInterface;
}

IOnlineTitleFilePtr FOnlineSubsystemSteamCore::GetTitleFileInterface() const
{
	return nullptr;
}

IOnlineEntitlementsPtr FOnlineSubsystemSteamCore::GetEntitlementsInterface() const
{
	return nullptr;
}

IOnlineEventsPtr FOnlineSubsystemSteamCore::GetEventsInterface() const
{
	return nullptr;
}

IOnlineAchievementsPtr FOnlineSubsystemSteamCore::GetAchievementsInterface() const
{
	return m_AchievementsInterface;
}

IOnlineSharingPtr FOnlineSubsystemSteamCore::GetSharingInterface() const
{
	return nullptr;
}

IOnlineUserPtr FOnlineSubsystemSteamCore::GetUserInterface() const
{
	return nullptr;
}

IOnlineMessagePtr FOnlineSubsystemSteamCore::GetMessageInterface() const
{
	return nullptr;
}

IOnlinePresencePtr FOnlineSubsystemSteamCore::GetPresenceInterface() const
{
	return m_PresenceInterface;
}

IOnlineChatPtr FOnlineSubsystemSteamCore::GetChatInterface() const
{
	return nullptr;
}

IOnlineStatsPtr FOnlineSubsystemSteamCore::GetStatsInterface() const
{
	return nullptr;
}

IOnlineTurnBasedPtr FOnlineSubsystemSteamCore::GetTurnBasedInterface() const
{
	return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemSteamCore::GetTournamentInterface() const
{
	return nullptr;
}

void FOnlineSubsystemSteamCore::QueueAsyncTask(FOnlineAsyncTask* AsyncTask)
{
	check(m_OnlineAsyncTaskThreadRunnable);
	m_OnlineAsyncTaskThreadRunnable->AddToInQueue(AsyncTask);
}

void FOnlineSubsystemSteamCore::QueueAsyncOutgoingItem(FOnlineAsyncItem* AsyncItem)
{
	check(m_OnlineAsyncTaskThreadRunnable);
	m_OnlineAsyncTaskThreadRunnable->AddToOutQueue(AsyncItem);
}

bool FOnlineSubsystemSteamCore::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_FOnlineSubsystemSteamCore_Tick);

	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}

	if (m_OnlineAsyncTaskThreadRunnable)
	{
		m_OnlineAsyncTaskThreadRunnable->GameTick();
	}

	if (m_SessionInterface.IsValid())
	{
		m_SessionInterface->Tick(DeltaTime);
	}

	if (m_VoiceInterface.IsValid() && m_bVoiceInterfaceInitialized)
	{
		m_VoiceInterface->Tick(DeltaTime);
	}

	if (m_AuthInterface.IsValid())
	{
		m_AuthInterface->Tick(DeltaTime);
	}

	return true;
}

bool FOnlineSubsystemSteamCore::Init()
{
	bool bRelaunchInSteam = false;
	int RelaunchAppId = 0;

	if (!ConfigureSteamCoreInitDevOptions(bRelaunchInSteam, RelaunchAppId))
	{
		LogSteamCoreWarn("Could not set up the steam environment! Falling back to another OSS.");
		return false;
	}

	const bool bIsServer = IsRunningDedicatedServer();
	bool bInitServerOnClient = false;
	GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bInitServerOnClient"), bInitServerOnClient, GEngineIni);
	const bool bAttemptServerInit = bIsServer || bInitServerOnClient;

	LogSteamCoreVerbose("Steam: Starting SteamWorks. Client [%d] Server [%d]", !bIsServer, bAttemptServerInit);

	const bool bClientInitSuccess = !bIsServer ? InitSteamworksClient(bRelaunchInSteam, RelaunchAppId) : true;
	const bool bServerInitSuccess = bAttemptServerInit ? (InitSteamworksServer()) : true;

	if (bClientInitSuccess && bServerInitSuccess)
	{
		m_OnlineAsyncTaskThreadRunnable = new FOnlineAsyncTaskManagerSteamCore(this);
		check(m_OnlineAsyncTaskThreadRunnable);
		m_OnlineAsyncTaskThread = FRunnableThread::Create(m_OnlineAsyncTaskThreadRunnable, *FString::Printf(TEXT("OnlineAsyncTaskThreadSteam %s"), *InstanceName.ToString()), 128 * 1024, TPri_Normal);
		check(m_OnlineAsyncTaskThread);
		LogSteamCoreVerbose("Created thread (ID:%d).", m_OnlineAsyncTaskThread->GetThreadID());

		m_SessionInterface = MakeShareable(new FOnlineSessionSteamCore(this));
		m_SessionInterface->CheckPendingSessionInvite();

		m_IdentityInterface = MakeShareable(new FOnlineIdentitySteamCore(this));

		m_PresenceInterface = MakeShareable(new FOnlinePresenceSteamCore(this));

		m_AuthInterfaceUtils = MakeShareable(new FOnlineAuthUtilsSteamCore());
		m_AuthInterface = MakeShareable(new FOnlineAuthSteamCore(this, m_AuthInterfaceUtils));

		if (!bIsServer)
		{
			m_FriendInterface = MakeShareable(new FOnlineFriendsSteamCore(this));
			m_UserCloudInterface = MakeShareable(new FOnlineUserCloudSteamCore(this));
			m_SharedCloudInterface = MakeShareable(new FOnlineSharedCloudSteamCore(this));
			m_LeaderboardsInterface = MakeShareable(new FOnlineLeaderboardsSteamCore(this));
			m_VoiceInterface = MakeShareable(new FOnlineVoiceSteamCore(this));
			m_ExternalUIInterface = MakeShareable(new FOnlineExternalUISteamCore(this));
#if WITH_STEAMCORE
			m_AchievementsInterface = MakeShareable(new FOnlineAchievementsSteamCore(this));
#endif
			m_EncryptedAppTicketInterface = MakeShareable(new FOnlineEncryptedAppTicketSteamCore(this));
			m_LeaderboardsInterface->CacheCurrentUsersStats();
		}
		else
		{
			m_VoiceInterface = MakeShareable(new FOnlineVoiceSteamCore(this));
		}
	}
	else
	{
		if (bClientInitSuccess)
		{
			LogSteamCoreWarn("Failed to initialize Steam, this could be due to a Steam server and client running on the same machine.");
		}
		Shutdown();
	}

	return bClientInitSuccess && bServerInitSuccess;
}

bool FOnlineSubsystemSteamCore::Shutdown()
{
	LogSteamCoreVerbose("OnlineSubsystemSteamCore::Shutdown()");

	FOnlineSubsystemImpl::Shutdown();

	if (m_OnlineAsyncTaskThread)
	{
		delete m_OnlineAsyncTaskThread;
		m_OnlineAsyncTaskThread = nullptr;
	}

	if (m_OnlineAsyncTaskThreadRunnable)
	{
		delete m_OnlineAsyncTaskThreadRunnable;
		m_OnlineAsyncTaskThreadRunnable = nullptr;
	}

	if (m_VoiceInterface.IsValid() && m_bVoiceInterfaceInitialized)
	{
		m_VoiceInterface->Shutdown();
	}

#define DESTRUCT_INTERFACE(Interface) \
	if (Interface.IsValid()) \
	{ \
		ensure(Interface.IsUnique()); \
		Interface = nullptr; \
	}

	DESTRUCT_INTERFACE(m_EncryptedAppTicketInterface);
	DESTRUCT_INTERFACE(m_AchievementsInterface);
	DESTRUCT_INTERFACE(m_ExternalUIInterface);
	DESTRUCT_INTERFACE(m_VoiceInterface);
	DESTRUCT_INTERFACE(m_LeaderboardsInterface);
	DESTRUCT_INTERFACE(m_SharedCloudInterface);
	DESTRUCT_INTERFACE(m_UserCloudInterface);
	DESTRUCT_INTERFACE(m_FriendInterface);
	DESTRUCT_INTERFACE(m_IdentityInterface);
	DESTRUCT_INTERFACE(m_AuthInterface);
	DESTRUCT_INTERFACE(m_AuthInterfaceUtils);
	DESTRUCT_INTERFACE(m_PingInterface);
	DESTRUCT_INTERFACE(m_SessionInterface);
	DESTRUCT_INTERFACE(m_PresenceInterface);

#undef DESTRUCT_INTERFACE

	ClearUserCloudFiles();

	ShutdownSteamworks();

#if !UE_BUILD_SHIPPING && !UE_BUILD_SHIPPING_WITH_EDITOR
	DeleteSteamAppIdFromDisk();
#endif // !UE_BUILD_SHIPPING && !UE_BUILD_SHIPPING_WITH_EDITOR

	return true;
}

bool FOnlineSubsystemSteamCore::IsEnabled() const
{
	if (m_bSteamworksClientInitialized || m_bSteamworksGameServerInitialized)
	{
		return true;
	}

	bool bEnableSteam = FOnlineSubsystemImpl::IsEnabled();
	if (bEnableSteam)
	{
#if UE_EDITOR
		if (bEnableSteam)
		{
			bEnableSteam = IsRunningDedicatedServer() || IsRunningGame();
		}
#endif
	}

	return bEnableSteam;
}

bool FOnlineSubsystemSteamCore::InitSteamworksClient(bool bRelaunchInSteam, int32 SteamAppId)
{
	m_bSteamworksClientInitialized = false;

	if (bRelaunchInSteam && SteamAppId != 0 && SteamAPI_RestartAppIfNecessary(SteamAppId))
	{
		if (FPlatformProperties::IsGameOnly() || FPlatformProperties::IsServerOnly())
		{
			LogSteamCoreVerbose("Game restarting within Steam client, exiting");
			FPlatformMisc::RequestExit(false);
		}

		return m_bSteamworksClientInitialized;
	}
	else
	{
		m_SteamAPIClientHandle = FSteamCoreSharedModule::Get().ObtainSteamClientInstanceHandle();
		m_bSteamworksClientInitialized = (m_SteamAPIClientHandle.IsValid() ? true : false);

#define GET_STEAMWORKS_INTERFACE(Interface) \
		if (Interface() == nullptr) \
		{ \
			LogSteamCoreWarn("Steamworks: %s() failed!", TEXT(#Interface)); \
			m_bSteamworksClientInitialized = false; \
		}
		GET_STEAMWORKS_INTERFACE(SteamUtils);
		GET_STEAMWORKS_INTERFACE(SteamUser);
		GET_STEAMWORKS_INTERFACE(SteamFriends);
		GET_STEAMWORKS_INTERFACE(SteamRemoteStorage);
		GET_STEAMWORKS_INTERFACE(SteamUserStats);
		GET_STEAMWORKS_INTERFACE(SteamMatchmakingServers);
		GET_STEAMWORKS_INTERFACE(SteamApps);
		GET_STEAMWORKS_INTERFACE(SteamNetworking);
		GET_STEAMWORKS_INTERFACE(SteamMatchmaking);

#undef GET_STEAMWORKS_INTERFACE
	}

	if (m_bSteamworksClientInitialized)
	{
		m_GameServerGamePort = m_SteamAPIClientHandle->GetGamePort();

		bool bIsSubscribed = true;
		if (FPlatformProperties::IsGameOnly() || FPlatformProperties::IsServerOnly())
		{
			bIsSubscribed = SteamApps()->BIsSubscribed();
		}

		if (bIsSubscribed)
		{
			LogSteamCoreVerbose("Steam User is subscribed %i", m_bSteamworksClientInitialized);
			if (SteamUtils())
			{
				m_SteamAppID = SteamUtils()->GetAppID();
				SteamUtils()->SetWarningMessageHook(SteamworksWarningMessageHook);
			}
		}
		else
		{
			LogSteamCoreError("Steam User is NOT subscribed, exiting.");
			m_bSteamworksClientInitialized = false;
			FPlatformMisc::RequestExit(false);
		}
	}

	LogSteamCoreVerbose("[AppId: %d] Client API initialized %i", GetSteamAppId(), m_bSteamworksClientInitialized);
	return m_bSteamworksClientInitialized;
}

bool FOnlineSubsystemSteamCore::InitSteamworksServer()
{
	m_SteamAPIServerHandle = FSteamCoreSharedModule::Get().ObtainSteamServerInstanceHandle();
	m_bSteamworksGameServerInitialized = (m_SteamAPIServerHandle.IsValid());

	if (m_bSteamworksGameServerInitialized)
	{
		m_GameServerGamePort = m_SteamAPIServerHandle->GetGamePort();
		m_GameServerQueryPort = m_SteamAPIServerHandle->GetQueryPort();

#define GET_STEAMWORKS_INTERFACE(Interface) \
		if (Interface() == nullptr) \
		{ \
			LogSteamCoreWarn("Steamworks: %s() failed!", TEXT(#Interface)); \
			m_bSteamworksGameServerInitialized = false; \
		}
		GET_STEAMWORKS_INTERFACE(SteamGameServer);
		GET_STEAMWORKS_INTERFACE(SteamGameServerStats);
		GET_STEAMWORKS_INTERFACE(SteamGameServerNetworking);
		GET_STEAMWORKS_INTERFACE(SteamGameServerUtils);

#undef GET_STEAMWORKS_INTERFACE
	}

	if (SteamGameServerUtils() != nullptr)
	{
		m_SteamAppID = SteamGameServerUtils()->GetAppID();
		SteamGameServerUtils()->SetWarningMessageHook(SteamworksWarningMessageHook);
	}

	LogSteamCoreVerbose("[AppId: %d] Game Server API initialized %i", GetSteamAppId(), m_bSteamworksGameServerInitialized);
	return m_bSteamworksGameServerInitialized;
}

void FOnlineSubsystemSteamCore::ShutdownSteamworks()
{
	if (m_bSteamworksGameServerInitialized)
	{
		m_SteamAPIServerHandle.Reset();
		if (m_SessionInterface.IsValid())
		{
			m_SessionInterface->m_GameServerSteamId = nullptr;
			m_SessionInterface->m_bSteamworksGameServerConnected = false;
		}
		m_bSteamworksGameServerInitialized = false;
	}

	if (m_bSteamworksClientInitialized)
	{
		m_SteamAPIClientHandle.Reset();
		m_bSteamworksClientInitialized = false;
	}
}

bool FOnlineSubsystemSteamCore::IsLocalPlayer(const FUniqueNetId& UniqueId) const
{
	ISteamUser* SteamUserPtr = SteamUser();
	return SteamUserPtr && SteamUserPtr->GetSteamID() == (const FUniqueNetIdSteam&)UniqueId;
}

FOnlineLeaderboardsSteamCore* FOnlineSubsystemSteamCore::GetInternalLeaderboardsInterface()
{
	return m_LeaderboardsInterface.Get();
}

FSteamUserCloudData* FOnlineSubsystemSteamCore::GetUserCloudEntry(const FUniqueNetId& UserId)
{
	FScopeLock ScopeLock(&m_UserCloudDataLock);
	for (int32 UserIdx = 0; UserIdx < m_UserCloudData.Num(); UserIdx++)
	{
		FSteamUserCloudData* UserMetadata = m_UserCloudData[UserIdx];
		if (*UserMetadata->UserId == UserId)
		{
			return UserMetadata; 
		}
	}

	const FUniqueNetIdSteam& SteamUserId = FUniqueNetIdSteam::Cast(UserId);
	FSteamUserCloudData* NewItem = new FSteamUserCloudData(SteamUserId);
	const int32 UserIdx = m_UserCloudData.Add(NewItem);
	return m_UserCloudData[UserIdx];
}

bool FOnlineSubsystemSteamCore::ClearUserCloudMetadata(const FUniqueNetId& UserId, const FString& FileName)
{
	if (FileName.Len() > 0)
	{
		FScopeLock ScopeLock(&m_UserCloudDataLock);
		if (FSteamUserCloudData* UserCloud = GetUserCloudEntry(UserId))
		{
			UserCloud->ClearMetadata(FileName);
		}
	}

	return true;
}

void FOnlineSubsystemSteamCore::ClearUserCloudFiles()
{
	FScopeLock ScopeLock(&m_UserCloudDataLock);
	for (int32 UserIdx = 0; UserIdx < m_UserCloudData.Num(); UserIdx++)
	{
		const FSteamUserCloudData* UserCloud = m_UserCloudData[UserIdx];
		delete UserCloud;
	}

	m_UserCloudData.Empty();
}

static FDelegateHandle GOnEnumerateUserFilesCompleteDelegateHandle;

TMap<IOnlineUserCloud*, FDelegateHandle> s_GPerCloudDeleteFromEnumerateUserFilesCompleteDelegateHandles;

static void DeleteFromEnumerateUserFilesComplete(bool bWasSuccessful, const FUniqueNetId& UserId)
{
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	check(SteamSubsystem);

	const IOnlineUserCloudPtr UserCloud = SteamSubsystem->GetUserCloudInterface();

	UserCloud->ClearOnEnumerateUserFilesCompleteDelegate_Handle(GOnEnumerateUserFilesCompleteDelegateHandle);
	s_GPerCloudDeleteFromEnumerateUserFilesCompleteDelegateHandles.Remove(UserCloud.Get());
	if (bWasSuccessful)
	{
		TArray<FCloudFileHeader> UserFiles;
		UserCloud->GetUserFileList(UserId, UserFiles);

		for (int32 Idx = 0; Idx < UserFiles.Num(); Idx++)
		{
			UserCloud->DeleteUserFile(UserId, UserFiles[Idx].FileName, true, true);
		}
	}
}

bool FOnlineSubsystemSteamCore::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar))
	{
		return true;
	}

	bool bWasHandled = false;
	if (FParse::Command(&Cmd, TEXT("DELETECLOUDFILES")))
	{
		const IOnlineUserCloudPtr UserCloud = GetUserCloudInterface();

		const FUniqueNetIdSteamRef SteamId = FUniqueNetIdSteam::Create(SteamUser()->GetSteamID());

		const FOnEnumerateUserFilesCompleteDelegate Delegate = FOnEnumerateUserFilesCompleteDelegate::CreateStatic(&DeleteFromEnumerateUserFilesComplete);
		s_GPerCloudDeleteFromEnumerateUserFilesCompleteDelegateHandles.Add(UserCloud.Get(), UserCloud->AddOnEnumerateUserFilesCompleteDelegate_Handle(Delegate));
		UserCloud->EnumerateUserFiles(*SteamId);
		bWasHandled = true;
	}
	else if (FParse::Command(&Cmd, TEXT("SYNCLOBBIES")))
	{
		if (m_SessionInterface.IsValid())
		{
			m_SessionInterface->SyncLobbies();
			bWasHandled = true;
		}
	}
	else if (FParse::Command(&Cmd, TEXT("AUTH")))
	{
		if (m_AuthInterface.IsValid())
		{
			bWasHandled = m_AuthInterface->Exec(Cmd);
		}
	}

	return bWasHandled;
}

FString FOnlineSubsystemSteamCore::GetAppId() const
{
	return FString::Printf(TEXT("%d"), GetSteamAppId());
}

FText FOnlineSubsystemSteamCore::GetOnlineServiceName() const
{
	return NSLOCTEXT("OnlineSubsystemSteamCore", "OnlineServiceName", "Steam");
}
#endif