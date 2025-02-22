/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Sessions/OnlineSessionAsyncServerSteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h" 

#define DEBUG_STEAM_FILTERS 1

#if WITH_STEAMCORE
namespace SteamCore
{
	FString GetBuildIdAsSteamKey(const FOnlineSessionSettings& SessionSettings)
	{
		return FString::Printf(TEXT("%s:%d"), UTF8_TO_TCHAR(STEAMKEY_BUILDUNIQUEID), SessionSettings.BuildUniqueId);
	}

	FString GetSessionFlagsAsString(const FOnlineSessionSettings& SessionSettings)
	{
		int32 BitShift = 0;
		int32 SessionFlags = 0;
		SessionFlags |= (SessionSettings.bShouldAdvertise ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bAllowJoinInProgress ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bIsLANMatch ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bIsDedicated ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bUsesStats ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bAllowInvites ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bUsesPresence ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bAllowJoinViaPresence ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bAllowJoinViaPresenceFriendsOnly ? 1 : 0) << BitShift++;
		SessionFlags |= (SessionSettings.bAntiCheatProtected ? 1 : 0) << BitShift++;

		return FString::FromInt(SessionFlags);
	}

	void GetServerKeyValuePairsFromSessionSettings(const FOnlineSessionSettings& SessionSettings, FSteamSessionKeyValuePairs& KeyValuePairs, EOnlineDataAdvertisementType::Type AdvertisementType)
	{
		FString KeyStr;
		for (FSessionSettings::TConstIterator It(SessionSettings.Settings); It; ++It)
		{
			FName Key = It.Key();
			const FOnlineSessionSetting& Setting = It.Value();
			if (Setting.AdvertisementType == AdvertisementType)
			{
				if (SessionKeyToSteamKey(Key, Setting.Data, KeyStr))
				{
					FString SettingStr = Setting.Data.ToString();
					if (!SettingStr.IsEmpty())
					{
						KeyValuePairs.Add(KeyStr, SettingStr);
					}
					else
					{
						LogSteamCoreWarn("Empty session setting %s %s of type %s", *Key.ToString(), *Setting.ToString(), EOnlineKeyValuePairDataType::ToString(Setting.Data.GetType()));
					}
				}
				else
				{
					LogSteamCoreWarn("Unsupported session setting %s %s of type %s", *Key.ToString(), *Setting.ToString(), EOnlineKeyValuePairDataType::ToString(Setting.Data.GetType()));
				}
			}
		}
	}

	void GetServerKeyValuePairsFromSessionInfo(FOnlineSessionInfoSteamCore* SessionInfo, FSteamSessionKeyValuePairs& KeyValuePairs)
	{
		const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
		FSteamConnectionMethod MethodUsed = SessionInfo->m_ConnectionMethod;

		if (SteamSubsystem && GConfig && MethodUsed == FSteamConnectionMethod::None)
		{
			bool bUseRelays = false;
			bool bIsDefaultSubsystem = false;
			GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bAllowP2PPacketRelay"), bUseRelays, GEngineIni);
			if (!GConfig->GetBool(TEXT("OnlineSubsystemSteamCore"), TEXT("bUseSteamNetworking"), bIsDefaultSubsystem, GEngineIni))
			{
				bIsDefaultSubsystem = true;
			}

			if (!SteamSubsystem->IsUsingSteamNetworking())
			{
				if (!bUseRelays)
				{
					MethodUsed = FSteamConnectionMethod::Direct;
				}
				else
				{
					MethodUsed = FSteamConnectionMethod::P2P;
				}
			}
			else
			{
				if (bIsDefaultSubsystem)
				{
					MethodUsed = FSteamConnectionMethod::P2P;
				}
				else
				{
					MethodUsed = FSteamConnectionMethod::Direct;
				}
			}

			SessionInfo->m_ConnectionMethod = MethodUsed;
		}

		KeyValuePairs.Add(STEAMKEY_CONNECTIONMETHOD, FString::Printf(TEXT("%s"), *SteamCore::LexToString(MethodUsed)));

		if (SessionInfo->m_SteamP2PAddr.IsValid())
		{
			KeyValuePairs.Add(STEAMKEY_P2PADDR, SessionInfo->m_SteamP2PAddr->ToString(false));
			KeyValuePairs.Add(STEAMKEY_P2PPORT, FString::FromInt(SessionInfo->m_SteamP2PAddr->GetPort()));
		}
	}

	void GetServerKeyValuePairsFromSession(const FOnlineSession* Session, FSteamSessionKeyValuePairs& KeyValuePairs)
	{
		const FUniqueNetIdSteam* SteamId = (FUniqueNetIdSteam*)(Session->OwningUserId.Get());
		const FString OwningUserIdStr = SteamId->ToString();
		KeyValuePairs.Add(STEAMKEY_OWNINGUSERID, OwningUserIdStr);
		KeyValuePairs.Add(STEAMKEY_OWNINGUSERNAME, Session->OwningUserName);
	}

	void UpdatePublishedSettings(UWorld* World, FNamedOnlineSession* Session)
	{
		LogSteamCoreVerbose("UpdatePublishedSettings");
	
		ISteamGameServer* SteamGameServerPtr = SteamGameServer();
		check(SteamGameServerPtr);

		FOnlineSessionSettings TempSessionSettings = Session->SessionSettings;

		FString ServerName = Session->OwningUserName;
		SteamGameServerPtr->SetServerName(TCHAR_TO_UTF8(*ServerName));

		int32 NumTotalSlots = Session->SessionSettings.NumPublicConnections + Session->SessionSettings.NumPrivateConnections;
		SteamGameServerPtr->SetMaxPlayerCount(NumTotalSlots);

		FString Region(TEXT(""));
		SteamGameServerPtr->SetRegion(TCHAR_TO_UTF8(*Region));

		// @TODO Password protected
		SteamGameServerPtr->SetPasswordProtected(false);

		FString MapName;
		if (TempSessionSettings.Get(SETTING_MAPNAME, MapName) && !MapName.IsEmpty())
		{
			SteamGameServerPtr->SetMapName(TCHAR_TO_UTF8(*MapName));
		}
		TempSessionSettings.Remove(SETTING_MAPNAME);

		int32 BotCount = 0;
		if (TempSessionSettings.Get(SETTING_NUMBOTS, BotCount))
		{
			SteamGameServerPtr->SetBotPlayerCount(BotCount);
		}
		TempSessionSettings.Remove(SETTING_NUMBOTS);

		if (World)
		{
			if (AGameStateBase const* const GameState = World->GetGameState())
			{
				for (int32 PlayerIdx = 0; PlayerIdx < GameState->PlayerArray.Num(); PlayerIdx++)
				{
					APlayerState const* const PlayerState = GameState->PlayerArray[PlayerIdx];
					if (PlayerState && PlayerState->GetUniqueId().IsValid())
					{
						CSteamID SteamId(*(uint64*)PlayerState->GetUniqueId()->GetBytes());
						SteamGameServerPtr->BUpdateUserData(SteamId, TCHAR_TO_UTF8(*PlayerState->GetPlayerName()), PlayerState->GetScore());
					}
				}
			}
		}

		FSteamSessionKeyValuePairs ViaOnlineServicePairs;
		GetServerKeyValuePairsFromSession(Session, ViaOnlineServicePairs);

		ViaOnlineServicePairs.Add(STEAMKEY_SESSIONFLAGS, GetSessionFlagsAsString(Session->SessionSettings));

		FString GameTagsString = GetBuildIdAsSteamKey(Session->SessionSettings);

		GetServerKeyValuePairsFromSessionSettings(TempSessionSettings, ViaOnlineServicePairs, EOnlineDataAdvertisementType::ViaOnlineService);

		FSteamSessionKeyValuePairs ViaPingPairs;
		GetServerKeyValuePairsFromSessionSettings(TempSessionSettings, ViaPingPairs, EOnlineDataAdvertisementType::ViaPingOnly);

		FSteamSessionKeyValuePairs ViaOnlineServicePingPairs;
		GetServerKeyValuePairsFromSessionSettings(TempSessionSettings, ViaOnlineServicePingPairs, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		for (FSteamSessionKeyValuePairs::TConstIterator It(ViaOnlineServicePairs); It; ++It)
		{
			LogSteamCoreVerbose("Master Server Game Tags (%s, %s)", *It.Key(), *It.Value());
			FString NewKey = FString::Printf(TEXT(",%s:%s"), *It.Key(), *It.Value());
			if (GameTagsString.Len() + NewKey.Len() < k_cbMaxGameServerTags)
			{
				GameTagsString += NewKey;
			}
			else
			{
				LogSteamCoreWarn("Server setting %s overflows Steam SetGameTags call", *NewKey);
			}
		}

		if (Session->SessionInfo.IsValid())
		{
			FOnlineSessionInfoSteamCore* SessionInfo = static_cast<FOnlineSessionInfoSteamCore*>(Session->SessionInfo.Get());
			GetServerKeyValuePairsFromSessionInfo(SessionInfo, ViaOnlineServicePairs);
		}

		ViaOnlineServicePairs.Append(ViaOnlineServicePingPairs);

		FString GameDataString;
		for (FSteamSessionKeyValuePairs::TConstIterator It(ViaOnlineServicePairs); It; ++It)
		{
			LogSteamCoreVerbose("Master Server Game Data (%s, %s)", *It.Key(), *It.Value());
			FString NewKey = FString::Printf(TEXT("%s:%s"), *It.Key(), *It.Value());

			if (GameDataString.Len() + NewKey.Len() < k_cbMaxGameServerGameData)
			{
				GameDataString += ((GameDataString.Len() > 0) ? "," : "") + NewKey;
			}
			else
			{
				LogSteamCoreWarn("Server setting %s overflows Steam SetGameData call", *NewKey);
			}
		}

		if (GameTagsString.Len() > 0 && GameTagsString.Len() < k_cbMaxGameServerTags)
		{
			LogSteamCoreVerbose("SetGameTags(%s)", *GameTagsString);
			SteamGameServerPtr->SetGameTags(TCHAR_TO_UTF8(*GameTagsString));
		}

		if (GameDataString.Len() > 0 && GameDataString.Len() < k_cbMaxGameServerGameData)
		{
			LogSteamCoreVerbose("SetGameData(%s)", *GameDataString);
			SteamGameServerPtr->SetGameData(TCHAR_TO_UTF8(*GameDataString));
		}

		SteamGameServerPtr->ClearAllKeyValues();
		ViaOnlineServicePairs.Append(ViaPingPairs);

		for (FSteamSessionKeyValuePairs::TConstIterator AuxKeyIt(ViaOnlineServicePairs); AuxKeyIt; ++AuxKeyIt)
		{
			LogSteamCoreVerbose("Pushing Server KVData (%s, %s)", *AuxKeyIt.Key(), *AuxKeyIt.Value());
			SteamGameServerPtr->SetKeyValue(TCHAR_TO_UTF8(*AuxKeyIt.Key()), TCHAR_TO_UTF8(*AuxKeyIt.Value()));
		}

		const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	
		if (SteamSubsystem && SteamSubsystem->GetPingInterface().IsValid() && SteamSubsystem->GetPingInterface()->IsUsingP2PRelays())
		{
			const FString PingData = SteamSubsystem->GetPingInterface()->GetHostPingData();
			if (!PingData.IsEmpty())
			{
				SteamGameServerPtr->SetKeyValue(STEAMKEY_P2PPING, TCHAR_TO_UTF8(*PingData));
			}
		}
	}
}

FString FOnlineAsyncTaskSteamCoreCreateServer::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreCreateServer bWasSuccessful: %d"), WasSuccessful());
}

void FOnlineAsyncTaskSteamCoreCreateServer::Tick()
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreCreateServer::Tick");
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (!m_bInit)
	{
		LogSteamCoreVerbose("Initializing Steam Server");
		ISteamGameServer* SteamGameServerPtr = SteamGameServer();
		const FNamedOnlineSession* Session = (SessionInt.IsValid()) ? SessionInt->GetNamedSession(m_SessionName) : nullptr;
		if (Session != nullptr && SteamGameServerPtr != nullptr)
		{
			LogSteamCoreVerbose("Setting Steam Server Parameters");
			
			const bool bWantsDedicated = Session->SessionSettings.bIsDedicated;

			FString GameServerProductName;
			FString GameServerGameDescription;
			
			int32 SteamAppId = 0;
			GConfig->GetInt(TEXT("OnlineSubsystemSteamCore"), TEXT("SteamAppId"), SteamAppId, GEngineIni);
			check(SteamAppId != 0);
			const FString SteamAppIdAsString = FString::Printf(TEXT("%d"), SteamAppId);
			
			if (!GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameServerProductName"), GameServerProductName, GEngineIni))
			{
				GameServerProductName = SteamAppIdAsString;
			}

			if (!GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("GameServerGameDescription"), GameServerGameDescription, GEngineIni))
			{
				GameServerGameDescription = SteamAppIdAsString;
			}
			
			SteamGameServerPtr->SetProduct(TCHAR_TO_UTF8(*GameServerProductName));
			SteamGameServerPtr->SetGameDescription(TCHAR_TO_UTF8(*GameServerGameDescription));
			SteamGameServerPtr->SetModDir(TCHAR_TO_UTF8(*GameServerProductName));
			SteamGameServerPtr->SetDedicatedServer(bWantsDedicated);

			if (!SteamGameServerPtr->BLoggedOn())
			{
				SteamGameServerPtr->LogOnAnonymous();
				LogSteamCoreVerbose("Login server to Steam");
			}

			SteamGameServerPtr->SetAdvertiseServerActive(true);

			LogSteamCoreVerbose("Starting Steam game server. Dedicated? %d Game Dir is: %s Product Name is: %s\nGame Desc is: %s", bWantsDedicated, *SteamAppIdAsString, *SteamAppIdAsString, *SteamAppIdAsString);

			LogSteamCoreVerbose("Steam Server Initialized");
			
			m_bInit = true;
		}
	}

	if (m_bInit && SessionInt->m_bSteamworksGameServerConnected && SessionInt->m_GameServerSteamId->IsValid() && SessionInt->m_bPolicyResponseReceived)
	{
		bIsComplete = true;
		bWasSuccessful = true;
	}
	else
	{
		if (GetElapsedTime() >= ASYNC_TASK_TIMEOUT)
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
}

void FOnlineAsyncTaskSteamCoreCreateServer::Finalize()
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreCreateServer::Finalize");
	
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (bWasSuccessful)
	{
		if (FNamedOnlineSession* Session = SessionInt->GetNamedSession(m_SessionName))
		{
			FOnlineSessionInfoSteamCore* NewSessionInfo = new FOnlineSessionInfoSteamCore(ESteamSession::AdvertisedSessionHost, *SessionInt->m_GameServerSteamId);
			NewSessionInfo->Init();

			ISteamGameServer* SteamGameServerPtr = SteamGameServer();
			check(SteamGameServerPtr);

			NewSessionInfo->m_SteamP2PAddr = ISocketSubsystem::Get()->GetLocalBindAddr(*GLog);
			NewSessionInfo->m_SteamP2PAddr->SetPort(Subsystem->GetGameServerGamePort());
			LogSteamCoreVerbose("Server SteamP2P IP: %s", *NewSessionInfo->m_SteamP2PAddr->ToString(true));

			NewSessionInfo->m_HostAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			NewSessionInfo->m_HostAddr->SetIp(SteamGameServerPtr->GetPublicIP().m_unIPv4);
			NewSessionInfo->m_HostAddr->SetPort(Subsystem->GetGameServerGamePort());
			LogSteamCoreVerbose("Server IP: %s", *NewSessionInfo->m_HostAddr->ToString(true));

			if (!Session->OwningUserId.IsValid())
			{
				check(Session->SessionSettings.bIsDedicated);
				Session->OwningUserId = SessionInt->m_GameServerSteamId;

				FString CustomDedicatedServerName = SessionInt->GetCustomDedicatedServerName();
				if (CustomDedicatedServerName.IsEmpty())
				{
					FString CustomSessionName;
					Session->SessionSettings.Get("OWNINGNAME", CustomSessionName);
					if (CustomSessionName.IsEmpty() == false)
					{
						Session->OwningUserName = CustomSessionName;
					} else
					{
						Session->OwningUserName = m_SessionName.ToString();
					}
				}
				else
				{
					Session->OwningUserName = CustomDedicatedServerName;
				}
			}

			bool bShouldUseAdvertise = true;
			const FOnlineAuthSteamCorePtr SteamAuth = Subsystem->GetAuthInterface();
			if (SteamAuth && SteamAuth.IsValid())
			{
				bShouldUseAdvertise = !SteamAuth->IsSessionAuthEnabled();
			}

			Session->SessionInfo = MakeShareable(NewSessionInfo);
			Session->SessionSettings.bAntiCheatProtected = SteamGameServerPtr->BSecure() != 0 ? true : false;

			Session->SessionState = EOnlineSessionState::Pending;

			UWorld* World = GetWorldForOnline(Subsystem->GetInstanceName());
			SteamCore::UpdatePublishedSettings(World, Session);

			SessionInt->RegisterLocalPlayers(Session);

			if (SteamUser() && bShouldUseAdvertise)
			{
				SteamUser()->AdvertiseGame(k_steamIDNonSteamGS, SteamGameServerPtr->GetPublicIP().m_unIPv4, Subsystem->GetGameServerGamePort());
			}
		}
		else
		{
			LogSteamCoreWarn("No session %s found to update with Steam backend", *m_SessionName.ToString());
		}
	}
	else
	{
		SessionInt->RemoveNamedSession(m_SessionName);
	}
}

void FOnlineAsyncTaskSteamCoreCreateServer::TriggerDelegates()
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreCreateServer::TriggerDelegates");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt && SessionInt.IsValid())
	{
		SessionInt->TriggerOnCreateSessionCompleteDelegates(m_SessionName, bWasSuccessful);
	}
}

FString FOnlineAsyncTaskSteamCoreUpdateServer::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreUpdateServer bWasSuccessful: %d Session: %s"), WasSuccessful(), *m_SessionName.ToString());
}

void FOnlineAsyncTaskSteamCoreUpdateServer::Tick()
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreUpdateServer::Tick");
	
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (FNamedOnlineSession* Session = SessionInt->GetNamedSession(m_SessionName))
	{
		const bool bUsesPresence = Session->SessionSettings.bUsesPresence;
		if (bUsesPresence != m_NewSessionSettings.bUsesPresence)
		{
			LogSteamCoreWarn("Can't change presence settings on existing session %s, ignoring.", *m_SessionName.ToString());
		}

		Session->SessionSettings = m_NewSessionSettings;
		Session->SessionSettings.bUsesPresence = bUsesPresence;

		if (m_bUpdateOnlineData)
		{
			UWorld* World = GetWorldForOnline(Subsystem->GetInstanceName());

			SteamCore::UpdatePublishedSettings(World, Session);
		}

		bWasSuccessful = true;
	}
	else
	{
		LogSteamCoreWarn("No session %s found to update with Steam backend", *m_SessionName.ToString());
	}

	bIsComplete = true;
}

void FOnlineAsyncTaskSteamCoreUpdateServer::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		SessionInt->TriggerOnUpdateSessionCompleteDelegates(m_SessionName, bWasSuccessful);
	}
}

FString FOnlineAsyncTaskSteamCoreLogoffServer::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreLogoffServer bWasSuccessful: %d"), WasSuccessful());
}

void FOnlineAsyncTaskSteamCoreLogoffServer::Tick()
{
	LogSteamCoreVerbose("");
	if (!m_bInit)
	{
		SteamGameServer()->SetAdvertiseServerActive(false);
		SteamGameServer()->LogOff();
		m_bInit = true;
	}

	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (!SessionInt->m_bSteamworksGameServerConnected && !SessionInt->m_GameServerSteamId.IsValid())
	{
		bIsComplete = true;
		bWasSuccessful = true;
	}
	else
	{
		if (GetElapsedTime() >= ASYNC_TASK_TIMEOUT)
		{
			SessionInt->m_bSteamworksGameServerConnected = false;
			SessionInt->m_GameServerSteamId = NULL;
			bIsComplete = true;
			bWasSuccessful = false;
		}
	}
}

bool FPendingSearchResultSteamCore::FillSessionFromServerRules()
{
	LogSteamCoreVerbose("FPendingSearchResultSteam::FillSessionFromServerRules");
	bool bSuccess = true;

	const TSharedPtr<FOnlineSessionInfoSteamCore> SessionInfo = MakeShareable(new FOnlineSessionInfoSteamCore(ESteamSession::AdvertisedSessionClient, *m_ServerId));
	const TSharedRef<FInternetAddrSteamCore> SteamP2PAddr = MakeShareable(new FInternetAddrSteamCore);

	FOnlineSession* Session = &m_PendingSearchResult.Session;

	int32 KeysFound = 0;
	int32 SteamAddrKeysFound = 0;

	for (FSteamSessionKeyValuePairs::TConstIterator It(m_ServerRules); It; ++It)
	{
		if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_SESSIONFLAGS) == 0)
		{
			int32 BitShift = 0;
			const int32 SessionFlags = FCString::Atoi(*It.Value());
			Session->SessionSettings.bShouldAdvertise = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bAllowJoinInProgress = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bIsLANMatch = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bIsDedicated = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bUsesStats = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bAllowInvites = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bUsesPresence = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bAllowJoinViaPresence = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bAllowJoinViaPresenceFriendsOnly = (SessionFlags & (1 << BitShift++)) ? true : false;
			Session->SessionSettings.bAntiCheatProtected = (SessionFlags & (1 << BitShift++)) ? true : false;
			KeysFound++;
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_OWNINGUSERID) == 0)
		{
			uint64 UniqueId = FCString::Atoi64(*It.Value());
			if (UniqueId != 0)
			{
				Session->OwningUserId = FUniqueNetIdSteam::Create(UniqueId);
				KeysFound++;
			}
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_OWNINGUSERNAME) == 0)
		{
			if (FCString::Strlen(*It.Value()) > 0)
			{
				Session->OwningUserName = It.Value();
				KeysFound++;
			}
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_CONNECTIONMETHOD) == 0)
		{
			SessionInfo->m_ConnectionMethod = SteamCore::ToConnectionMethod(It.Value());
			++KeysFound;
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_P2PPING) == 0)
		{
			SessionInfo->m_SteamPingHostData = UTF8_TO_TCHAR(*It.Value());
			++KeysFound;
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_P2PADDR) == 0)
		{
			FString KeyValue = It.Value();
			KeyValue.RemoveFromStart(STEAM_URL_PREFIX);

			uint64 SteamAddr = FCString::Atoi64(*KeyValue);
			if (SteamAddr != 0)
			{
				SteamP2PAddr->m_SteamId = FUniqueNetIdSteam::Create(SteamAddr);
				SteamAddrKeysFound++;
			}
		}
		else if (FCStringAnsi::Stricmp(TCHAR_TO_ANSI(*It.Key()), STEAMKEY_P2PPORT) == 0)
		{
			const int32 Port = FCString::Atoi(*It.Value());
			SteamP2PAddr->SetPort(Port);
			SteamAddrKeysFound++;
		}
		else
		{
			FName NewKey;
			FOnlineSessionSetting NewSetting;
			if (SteamKeyToSessionSetting(*It.Key(), TCHAR_TO_ANSI(*It.Value()), NewKey, NewSetting))
			{
				Session->SessionSettings.Set(NewKey, NewSetting);
			}
			else
			{
				//bSuccess = false;
				LogSteamCoreWarn("Failed to parse setting from key %s value %s", *It.Key(), *It.Value());
			}
		}
	}

	LogSteamCoreVerbose("FPendingSearchResultSteam::FillSessionFromServerRules: bSuccess: %d, KeysFound: %d(%d), SteamAddrKeysFound: %d, m_HostAddr: %d", bSuccess, KeysFound, STEAMKEY_NUMREQUIREDSERVERKEYS, SteamAddrKeysFound, m_HostAddr.IsValid());
	//if (bSuccess && (KeysFound == STEAMKEY_NUMREQUIREDSERVERKEYS) && (SteamAddrKeysFound == 2 || (m_HostAddr.IsValid())))
	if (bSuccess && (SteamAddrKeysFound == 2 || (m_HostAddr.IsValid())))
	{
		SessionInfo->m_HostAddr = m_HostAddr;
		SessionInfo->m_SteamP2PAddr = SteamP2PAddr;

		Session->SessionInfo = SessionInfo;
		return true;
	}

	return false;
}

void FPendingSearchResultSteamCore::RulesResponded(const char* pchRule, const char* pchValue)
{
	LogSteamCoreWarn("Rules response %s %s", UTF8_TO_TCHAR(pchRule), UTF8_TO_TCHAR(pchValue));
	m_ParentQuery->m_ElapsedTime = 0.0f;
	m_ServerRules.Add(UTF8_TO_TCHAR(pchRule), UTF8_TO_TCHAR(pchValue));
}

void FPendingSearchResultSteamCore::RulesFailedToRespond()
{
	LogSteamCoreWarn("Rules failed to respond for server");
	m_ParentQuery->m_ElapsedTime = 0.0f;
	//RemoveSelf();
}

void FPendingSearchResultSteamCore::RulesRefreshComplete()
{
	LogSteamCoreVerbose("FPendingSearchResultSteam::RulesRefreshComplete: Rules refresh complete");
	m_ParentQuery->m_ElapsedTime = 0.0f;

	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(m_ParentQuery->Subsystem->GetSessionInterface());
	if (SessionInt.IsValid() && SessionInt->m_CurrentSessionSearch.IsValid() && SessionInt->m_CurrentSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		if (FillSessionFromServerRules())
		{
			FOnlineSessionSearchResult* SearchResult = new(m_ParentQuery->m_SearchSettings->SearchResults) FOnlineSessionSearchResult(m_PendingSearchResult);
			SearchResult->Session.SessionInfo = m_PendingSearchResult.Session.SessionInfo;
			if (!SearchResult->IsValid())
			{
				LogSteamCoreVerbose("SearchResult was not valid, removing result.");
				m_ParentQuery->m_SearchSettings->SearchResults.RemoveAtSwap(m_ParentQuery->m_SearchSettings->SearchResults.Num() - 1);
			}
		}
		else {
			LogSteamCoreVerbose("FillSessionFromServerRules() failed");
		}
	}

	RemoveSelf();
}

void FPendingSearchResultSteamCore::RemoveSelf() const
{
	for (int32 SearchIdx = 0; SearchIdx < m_ParentQuery->m_PendingSearchResults.Num(); SearchIdx++)
	{
		if (m_ParentQuery->m_PendingSearchResults[SearchIdx].m_ServerId == m_ServerId)
		{
			m_ParentQuery->m_PendingSearchResults.RemoveAtSwap(SearchIdx);
			break;
		}
	}
}

void FPendingSearchResultSteamCore::CancelQuery()
{
	if (m_ServerQueryHandle != HSERVERQUERY_INVALID)
	{
		SteamMatchmakingServers()->CancelServerQuery(m_ServerQueryHandle);
		m_ServerQueryHandle = HSERVERQUERY_INVALID;
	}
}


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6385)
#endif

void FOnlineAsyncTaskSteamCoreFindServerBase::CreateQuery(MatchMakingKeyValuePair_t** OutFilter, int32& NumFilters)
{
	FOnlineSearchSettings TempSearchSettings = m_SearchSettings->QuerySettings;
	const int32 MaxFilters = TempSearchSettings.SearchParams.Num() + 1;

	*OutFilter = new MatchMakingKeyValuePair_t[MaxFilters];
	MatchMakingKeyValuePair_t* Filters = *OutFilter;

	const int32 KeySize = sizeof(Filters[0].m_szKey);
	const int32 ValueSize = sizeof(Filters[0].m_szValue);

	NumFilters = 0;
//	FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "gamedir", KeySize);
//	FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, STEAM_GAME_DIR, ValueSize);
//NumFilters++;

	FString MapName;
	if (TempSearchSettings.Get(SETTING_MAPNAME, MapName) && !MapName.IsEmpty())
	{
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "map", KeySize);
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, TCHAR_TO_ANSI(*MapName), ValueSize);
		NumFilters++;
	}
	TempSearchSettings.SearchParams.Remove(SETTING_MAPNAME);

	FString HostIp;
	if (TempSearchSettings.Get(SEARCH_STEAM_HOSTIP, HostIp) && !HostIp.IsEmpty())
	{
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "gameaddr", KeySize);
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, TCHAR_TO_ANSI(*HostIp), ValueSize);
		NumFilters++;
	}
	TempSearchSettings.SearchParams.Remove(SEARCH_STEAM_HOSTIP);

	int32 DedicatedOnly = 0;
	if (TempSearchSettings.Get(SEARCH_DEDICATED_ONLY, DedicatedOnly) && DedicatedOnly != 0)
	{
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "dedicated", KeySize);
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, "true", ValueSize);
		NumFilters++;
	}
	TempSearchSettings.SearchParams.Remove(SEARCH_DEDICATED_ONLY);

	int32 SecureOnly = 0;
	if (TempSearchSettings.Get(SEARCH_SECURE_SERVERS_ONLY, SecureOnly) && SecureOnly != 0)
	{
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "secure", KeySize);
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, "true", ValueSize);
		NumFilters++;
	}
	TempSearchSettings.SearchParams.Remove(SEARCH_SECURE_SERVERS_ONLY);

	int32 EmptyOnly = 0;
	if (TempSearchSettings.Get(SEARCH_EMPTY_SERVERS_ONLY, EmptyOnly) && EmptyOnly != 0)
	{
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szKey, "noplayers", KeySize);
		FCStringAnsi::Strncpy(Filters[NumFilters].m_szValue, "true", ValueSize);
		NumFilters++;
	}
	TempSearchSettings.SearchParams.Remove(SEARCH_EMPTY_SERVERS_ONLY);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

void FOnlineAsyncTaskSteamCoreFindServerBase::ParseSearchResult(class gameserveritem_t* ServerDetails)
{
	const TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	ServerAddr->SetIp(ServerDetails->m_NetAdr.GetIP());
	ServerAddr->SetPort(ServerDetails->m_NetAdr.GetConnectionPort());
	const int32 ServerQueryPort = ServerDetails->m_NetAdr.GetQueryPort();

	LogSteamCoreVerbose("Server response IP:%s", *ServerAddr->ToString(false));
	if (ServerDetails->m_bHadSuccessfulResponse)
	{
		const FString GameTags(UTF8_TO_TCHAR(ServerDetails->m_szGameTags));

		TArray<FString> TagArray;
		GameTags.ParseIntoArray(TagArray, TEXT(","), true);

		FPendingSearchResultSteamCore* NewPendingSearch = new FPendingSearchResultSteamCore(this);
		m_PendingSearchResults.Add(NewPendingSearch);
		NewPendingSearch->m_ServerId = FUniqueNetIdSteam::Create(ServerDetails->m_steamID);
		NewPendingSearch->m_HostAddr = ServerAddr;

		FOnlineSessionSearchResult* NewSearchResult = &NewPendingSearch->m_PendingSearchResult;
		NewSearchResult->PingInMs = FMath::Clamp(ServerDetails->m_nPing, 0, MAX_QUERY_PING);

		FOnlineSession* NewSession = &NewSearchResult->Session;

		NewSession->OwningUserName = UTF8_TO_TCHAR(ServerDetails->GetName());

		NewSession->NumOpenPublicConnections = ServerDetails->m_nMaxPlayers - ServerDetails->m_nPlayers;
		NewSession->NumOpenPrivateConnections = 0;

		NewSession->SessionSettings.NumPublicConnections = ServerDetails->m_nMaxPlayers;
		NewSession->SessionSettings.NumPrivateConnections = 0;
		NewSession->SessionSettings.bAntiCheatProtected = ServerDetails->m_bSecure ? true : false;
		NewSession->SessionSettings.Set(SETTING_MAPNAME, FString(UTF8_TO_TCHAR(ServerDetails->m_szMap)), EOnlineDataAdvertisementType::ViaOnlineService);

		NewPendingSearch->m_ServerQueryHandle = m_SteamMatchmakingServersPtr->ServerRules(ServerDetails->m_NetAdr.GetIP(), ServerQueryPort, NewPendingSearch);
		if (NewPendingSearch->m_ServerQueryHandle == HSERVERQUERY_INVALID)
		{
			m_PendingSearchResults.RemoveAtSwap(m_PendingSearchResults.Num() - 1);
		}
	}
}

void FOnlineAsyncTaskSteamCoreFindServerBase::Tick()
{
	const ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		m_SteamMatchmakingServersPtr = SteamMatchmakingServers();
		check(m_SteamMatchmakingServersPtr);

		int32 NumFilters = 0;
		MatchMakingKeyValuePair_t* Filters = nullptr;
		CreateQuery(&Filters, NumFilters);

#if DEBUG_STEAM_FILTERS
		for (int32 FilterIdx = 0; FilterIdx < NumFilters; FilterIdx++)
		{
			LogSteamCoreVerbose(" \"%s\" \"%s\" ", UTF8_TO_TCHAR(Filters[FilterIdx].m_szKey), UTF8_TO_TCHAR(Filters[FilterIdx].m_szValue));
		}
#endif

		if (m_SearchSettings->MaxSearchResults <= 0)
		{
			LogSteamCoreWarn("FOnlineAsyncTaskSteamCoreFindServerBase::Tick - SearchSettings->MaxSearchResults should be greater than 0, but it is currently %d. No search results will be found.", m_SearchSettings->MaxSearchResults);
		}

		if (m_SearchSettings->bIsLanQuery)
		{
			m_ServerListRequestHandle = m_SteamMatchmakingServersPtr->RequestLANServerList(Subsystem->GetSteamAppId(), this);
		}
		else
		{
			m_ServerListRequestHandle = m_SteamMatchmakingServersPtr->RequestInternetServerList(Subsystem->GetSteamAppId(), &Filters, NumFilters, this);
		}

		if (m_ServerListRequestHandle == nullptr)
		{
			bIsComplete = true;
			bWasSuccessful = false;
		}

		m_PendingSearchResults.Empty(m_SearchSettings->MaxSearchResults);

		delete [] Filters;
		m_bInit = true;
	}

	m_ElapsedTime += 1.0f / 16.0f;

	const bool bReachedSearchLimit = (m_SearchSettings->SearchResults.Num() >= m_SearchSettings->MaxSearchResults) ? true : false;
	const bool bTimedOut = (m_ElapsedTime >= ASYNC_TASK_TIMEOUT) ? true : false;
	const bool bServerSearchComplete = (m_bServerRefreshComplete && m_PendingSearchResults.Num() == 0) ? true : false;
	if (bReachedSearchLimit || bTimedOut || bServerSearchComplete)
	{
		bIsComplete = true;
		bWasSuccessful = true;
	}

	if (bIsComplete)
	{
		if (m_ServerListRequestHandle != nullptr)
		{
			m_SteamMatchmakingServersPtr->CancelQuery(m_ServerListRequestHandle);
			m_SteamMatchmakingServersPtr->ReleaseRequest(m_ServerListRequestHandle);
			m_ServerListRequestHandle = nullptr;
		}

		for (int32 PendingIdx = 0; PendingIdx < m_PendingSearchResults.Num(); ++PendingIdx)
		{
			m_PendingSearchResults[PendingIdx].CancelQuery();
		}
		m_PendingSearchResults.Empty();
	}
}

void FOnlineAsyncTaskSteamCoreFindServerBase::ServerResponded(HServerListRequest Request, int iServer)
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreFindServerBase::ServerResponded %d", iServer);
	m_ElapsedTime = 0.0f;

	gameserveritem_t* Server = m_SteamMatchmakingServersPtr->GetServerDetails(Request, iServer);
	if (Server != nullptr)
	{
		const TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		ServerAddr->SetIp(Server->m_NetAdr.GetIP());
		ServerAddr->SetPort(Server->m_NetAdr.GetConnectionPort());

		LogSteamCoreWarn("FOnlineAsyncTaskSteamCoreFindServerBase::ServerResponded IP:%s", *ServerAddr->ToString(false));
		
		ParseSearchResult(Server);
	}
}

void FOnlineAsyncTaskSteamCoreFindServerBase::ServerFailedToRespond(HServerListRequest Request, int iServer)
{
	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreFindServerBase::ServerFailedToRespond %d", iServer);
	
	m_ElapsedTime = 0.0f;

	gameserveritem_t* Server = m_SteamMatchmakingServersPtr->GetServerDetails(Request, iServer);
	if (Server != nullptr)
	{
		const TSharedRef<FInternetAddr> ServerAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

		ServerAddr->SetIp(Server->m_NetAdr.GetIP());
		ServerAddr->SetPort(Server->m_NetAdr.GetConnectionPort());

		LogSteamCoreWarn("FOnlineAsyncTaskSteamCoreFindServerBase::ServerFailedToRespond IP:%s", *ServerAddr->ToString(false));

		ParseSearchResult(Server);
	}
}

void FOnlineAsyncTaskSteamCoreFindServerBase::RefreshComplete(HServerListRequest Request, EMatchMakingServerResponse Response)
{
	LogSteamCoreVerbose("Server query complete %s", *SteamCore::SteamMatchMakingServerResponseString(Response));
	m_bServerRefreshComplete = true;
	m_ElapsedTime = 0.0f;
}

void FOnlineAsyncTaskSteamCoreFindServerBase::Finalize()
{
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

	m_SearchSettings->SearchState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
	if (bWasSuccessful)
	{
		if (m_SearchSettings->SearchResults.Num() > 0)
		{
			m_SearchSettings->SortSearchResults();
		}
	}

	if (SessionInt->m_CurrentSessionSearch.IsValid() && m_SearchSettings == SessionInt->m_CurrentSessionSearch)
	{
		SessionInt->m_CurrentSessionSearch = nullptr;
	}
}

FString FOnlineAsyncTaskSteamCoreFindServerForInviteSession::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindServerForInvite bWasSuccessful: %d Results: %d"), WasSuccessful(), m_SearchSettings->SearchResults.Num());
}

void FOnlineAsyncTaskSteamCoreFindServerForInviteSession::TriggerDelegates()
{
	if (m_FindServerInviteCompleteWithUserIdDelegates.IsBound() && m_LocalUserNum >= 0)
	{
		if (bWasSuccessful && m_SearchSettings->SearchResults.Num() > 0)
		{
			m_FindServerInviteCompleteWithUserIdDelegates.Broadcast(bWasSuccessful, m_LocalUserNum, FUniqueNetIdSteam::Create(SteamUser()->GetSteamID()), m_SearchSettings->SearchResults[0]);
		}
		else
		{
			const FOnlineSessionSearchResult EmptyResult;
			m_FindServerInviteCompleteWithUserIdDelegates.Broadcast(bWasSuccessful, m_LocalUserNum, FUniqueNetIdSteam::Create(SteamUser()->GetSteamID()), EmptyResult);
		}
	}
}

FString FOnlineAsyncTaskSteamCoreFindServerForFriendSession::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindServerForFriend bWasSuccessful: %d Results: %d"), WasSuccessful(), m_SearchSettings->SearchResults.Num());
}

void FOnlineAsyncTaskSteamCoreFindServerForFriendSession::TriggerDelegates()
{
	if (m_FindServerInviteCompleteDelegates.IsBound() && m_LocalUserNum >= 0)
	{
		if (bWasSuccessful && m_SearchSettings->SearchResults.Num() > 0)
		{
			m_FindServerInviteCompleteDelegates.Broadcast(m_LocalUserNum, bWasSuccessful, m_SearchSettings->SearchResults);
		}
		else
		{
			const TArray<FOnlineSessionSearchResult> EmptyResult;
			m_FindServerInviteCompleteDelegates.Broadcast(m_LocalUserNum, bWasSuccessful, EmptyResult);
		}
	}
}

FString FOnlineAsyncTaskSteamCoreFindServers::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindServers bWasSuccessful: %d Results: %d"), WasSuccessful(), m_SearchSettings->SearchResults.Num());
}

void FOnlineAsyncTaskSteamCoreFindServers::TriggerDelegates()
{
	if (m_FindServersCompleteDelegates.IsBound())
	{
		m_FindServersCompleteDelegates.Broadcast(bWasSuccessful);
	}
}

void FOnlineAsyncEventSteamCoreInviteAccepted::Finalize()
{
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (SessionInt.IsValid() && !SessionInt->m_CurrentSessionSearch.IsValid())
	{
		const TSharedRef<FOnlineSessionSearch> SearchSettings = MakeShareable(new FOnlineSessionSearch());
		SessionInt->m_CurrentSessionSearch = SearchSettings;
		SessionInt->m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::InProgress;

		TCHAR ParsedURL[1024];
		if (!FParse::Value(*m_ConnectionURL, TEXT("SteamConnectIP="), ParsedURL, UE_ARRAY_COUNT(ParsedURL)))
		{
			LogSteamCoreWarn("FOnlineAsyncEventSteamInviteAccepted: Failed to parse connection URL");
			return;
		}

		int32 Port = 0;
		if (TCHAR* PortToken = FCString::Strchr(ParsedURL, ':'))
		{
			Port = FCString::Atoi(PortToken + 1);
			PortToken[0] = '\0';
		}

		Port = (Port > 0) ? Port : Subsystem->GetGameServerGamePort();

		const TSharedPtr<FInternetAddr> IpAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetAddressFromString(ParsedURL);
		if (IpAddr.IsValid())
		{
			SessionInt->m_CurrentSessionSearch->QuerySettings.Set(FName(SEARCH_STEAM_HOSTIP), IpAddr->ToString(false), EOnlineComparisonOp::Equals);
			FOnlineAsyncTaskSteamCoreFindServerForInviteSession* NewTask = new FOnlineAsyncTaskSteamCoreFindServerForInviteSession(Subsystem, SearchSettings, m_LocalUserNum, SessionInt->OnSessionUserInviteAcceptedDelegates);
			Subsystem->QueueAsyncTask(NewTask);
		}
	}
	else
	{
		LogSteamCoreWarn("Invalid session or search already in progress when accepting invite. Ignoring invite request.");
	}
}
#endif