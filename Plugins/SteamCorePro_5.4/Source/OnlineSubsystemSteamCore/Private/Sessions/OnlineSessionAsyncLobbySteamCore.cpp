/**
* Copyright (C) 2017-2024 eelDev AB
*
*/

#include "Sessions/OnlineSessionAsyncLobbySteamCore.h"
#include "OnlineSubsystemSteamCorePrivatePCH.h"

#if WITH_STEAMCORE
namespace SteamCore
{
	bool FillSessionFromLobbyData(const FOnlineSubsystemSteamCore* SteamSubsystem, const FUniqueNetIdSteam& LobbyId, FOnlineSession& Session, FOnlineSessionSearchResult* SearchData)
	{
		LogSteamCoreVerbose("");
		bool bSuccess = true;

		ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();
		check(SteamMatchmakingPtr);

		Session.SessionSettings.Settings.Empty();

		const TSharedPtr<FOnlineSessionInfoSteamCore> SessionInfo = MakeShared<FOnlineSessionInfoSteamCore>(ESteamSession::LobbySession, LobbyId);
		const TSharedRef<FInternetAddr> HostAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		const TSharedRef<FInternetAddrSteamCore> SteamP2PAddr = MakeShareable(new FInternetAddrSteamCore);

		int32 KeysFound = 0;
		int32 HostKeysFound = 0;
		int32 SteamAddrKeysFound = 0;

		const int32 LobbyDataBufferSize = 2000;

		const int32 LobbyDataCount = SteamMatchmakingPtr->GetLobbyDataCount(LobbyId);
		ANSICHAR Value[LobbyDataBufferSize];
		ANSICHAR Key[LobbyDataBufferSize];

		for (int32 LobbyDataIdx = 0; LobbyDataIdx < LobbyDataCount; LobbyDataIdx++)
		{
			if (!SteamMatchmakingPtr->GetLobbyDataByIndex(LobbyId, LobbyDataIdx, Key, LobbyDataBufferSize, Value, LobbyDataBufferSize))
			{
				KeysFound = 0;
				break;
			}

			if (FCStringAnsi::Stricmp(Key, STEAMKEY_NUMPUBLICCONNECTIONS) == 0)
			{
				Session.SessionSettings.NumPublicConnections = FCString::Atoi(ANSI_TO_TCHAR(Value));
				KeysFound++;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_NUMPRIVATECONNECTIONS) == 0)
			{
				Session.SessionSettings.NumPrivateConnections = FCString::Atoi(ANSI_TO_TCHAR(Value));
				KeysFound++;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_SESSIONFLAGS) == 0)
			{
				int32 BitShift = 0;
				const int32 SessionFlags = FCString::Atoi(ANSI_TO_TCHAR(Value));
				Session.SessionSettings.bShouldAdvertise = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bAllowJoinInProgress = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bIsLANMatch = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bIsDedicated = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bUsesStats = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bAllowInvites = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bUsesPresence = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bAllowJoinViaPresence = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bAllowJoinViaPresenceFriendsOnly = (SessionFlags & (1 << BitShift++)) ? true : false;
				Session.SessionSettings.bAntiCheatProtected = (SessionFlags & (1 << BitShift++)) ? true : false;
				KeysFound++;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_BUILDUNIQUEID) == 0)
			{
				Session.SessionSettings.BuildUniqueId = FCString::Atoi(ANSI_TO_TCHAR(Value));
				KeysFound++;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_OWNINGUSERID) == 0)
			{
				uint64 UniqueId = FCString::Atoi64(ANSI_TO_TCHAR(Value));
				if (UniqueId != 0)
				{
					Session.OwningUserId = FUniqueNetIdSteam::Create(UniqueId);
					KeysFound++;
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_OWNINGUSERNAME) == 0)
			{
				if (FCString::Strlen(ANSI_TO_TCHAR(Value)) > 0)
				{
					Session.OwningUserName = Value;
					KeysFound++;
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_NUMOPENPRIVATECONNECTIONS) == 0)
			{
				Session.NumOpenPrivateConnections = FCString::Atoi(ANSI_TO_TCHAR(Value));
				KeysFound++;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_NUMOPENPUBLICCONNECTIONS) == 0)
			{
				Session.NumOpenPublicConnections = FCString::Atoi(ANSI_TO_TCHAR(Value));
				KeysFound++;
			}
			else if (FCStringAnsi::Strcmp(Key, STEAMKEY_P2PPING) == 0)
			{
				if (SteamSubsystem && SearchData != nullptr && SteamSubsystem->GetPingInterface().IsValid())
				{
					const int32 PingResult = SteamSubsystem->GetPingInterface()->GetPingFromHostData(ANSI_TO_TCHAR(Value));
					if (PingResult >= 0)
					{
						SearchData->PingInMs = PingResult;
					}
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_HOSTIP) == 0)
			{
				const uint32 HostIp = FCString::Atoi(ANSI_TO_TCHAR(Value));
				if (HostIp != 0)
				{
					HostAddr->SetIp(HostIp);
					HostKeysFound++;
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_HOSTPORT) == 0)
			{
				const int32 Port = FCString::Atoi(ANSI_TO_TCHAR(Value));
				if (Port != 0)
				{
					HostAddr->SetPort(Port);
					HostKeysFound++;
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_CONNECTIONMETHOD) == 0)
			{
				SessionInfo->m_ConnectionMethod = SteamCore::ToConnectionMethod(ANSI_TO_TCHAR(Value));
				++KeysFound;
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_P2PADDR) == 0)
			{
				FString KeyValue = ANSI_TO_TCHAR(Value);
				KeyValue.RemoveFromStart(STEAM_URL_PREFIX);

				uint64 SteamAddr = FCString::Atoi64(*KeyValue);
				if (SteamAddr != 0)
				{
					SteamP2PAddr->m_SteamId = FUniqueNetIdSteam::Create(SteamAddr);
					SteamAddrKeysFound++;
				}
			}
			else if (FCStringAnsi::Stricmp(Key, STEAMKEY_P2PPORT) == 0)
			{
				const int32 Port = FCString::Atoi(ANSI_TO_TCHAR(Value));
				SteamP2PAddr->SetPort(Port);
				SteamAddrKeysFound++;
			}
			else
			{
				FName NewKey;
				FOnlineSessionSetting NewSetting;
				if (SteamKeyToSessionSetting(ANSI_TO_TCHAR(Key), Value, NewKey, NewSetting))
				{
					Session.SessionSettings.Set(NewKey, NewSetting);
				}
				else
				{
					bSuccess = false;
					LogSteamCoreWarn("Failed to parse setting from key %s value %s", ANSI_TO_TCHAR(Key), ANSI_TO_TCHAR(Value));
				}
			}
		}

		LogSteamCoreVerbose("Join Session bSuccess: (%d), KeysFound: (%d), HostKeysFound: (%d), SteamAddrKeysFound: (%d)", bSuccess, KeysFound, HostKeysFound, SteamAddrKeysFound);

		if (bSuccess && (KeysFound == STEAMKEY_NUMREQUIREDLOBBYKEYS) && (HostKeysFound == 2 || SteamAddrKeysFound == 2))
		{
			const int32 BuildUniqueId = GetBuildUniqueId();
			
			LogSteamCoreVerbose("Unique Build Id: (%d), Session Build Id: (%d)", BuildUniqueId, Session.SessionSettings.BuildUniqueId);
			
			if (Session.SessionSettings.BuildUniqueId == BuildUniqueId)
			{
				if (HostKeysFound == 2)
				{
					SessionInfo->m_HostAddr = HostAddr;
				}

				if (SteamAddrKeysFound == 2)
				{
					SessionInfo->m_SteamP2PAddr = SteamP2PAddr;
				}

				Session.SessionInfo = SessionInfo;
				return true;
			}
			else
			{
				LogSteamCoreWarn("Removed incompatible build: ServerBuildUniqueId = 0x%08x, GetBuildUniqueId() = 0x%08x", Session.SessionSettings.BuildUniqueId, BuildUniqueId);
			}
		}

		return false;
	}

	bool FillMembersFromLobbyData(const FUniqueNetIdSteam& LobbyId, FNamedOnlineSession& Session)
	{
		LogSteamCoreVerbose("");
		bool bSuccess = true;

		ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();
		check(SteamMatchmakingPtr);

		const int32 LobbyMemberCount = SteamMatchmakingPtr->GetNumLobbyMembers(LobbyId);
		const int32 MaxLobbyMembers = SteamMatchmakingPtr->GetLobbyMemberLimit(LobbyId);
		if (MaxLobbyMembers > 0)
		{
			Session.NumOpenPublicConnections = MaxLobbyMembers - LobbyMemberCount;

			if (SteamMatchmakingPtr->GetLobbyOwner(LobbyId) == SteamUser()->GetSteamID())
			{
				bool bLobbyJoinable = Session.SessionSettings.bAllowJoinInProgress && (LobbyMemberCount < MaxLobbyMembers);

				LogSteamCoreVerbose("Updating lobby joinability to %s.", bLobbyJoinable ? TEXT("true") : TEXT("false"));
				if (!SteamMatchmakingPtr->SetLobbyJoinable(LobbyId, bLobbyJoinable))
				{
					LogSteamCoreWarn("Failed to update lobby joinability.");
					bSuccess = false;
				}
			}
		}

		return bSuccess;
	}
}

FString FOnlineAsyncTaskSteamCoreCreateLobby::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreCreateLobby bWasSuccessful: %d LobbyId: %llu LobbyType: %d Result: %s"), WasSuccessful(), m_CallbackResults.m_ulSteamIDLobby, (int32)m_LobbyType, *SteamCore::SteamResultString(m_CallbackResults.m_eResult));
}

void FOnlineAsyncTaskSteamCoreCreateLobby::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		m_CallbackHandle = SteamMatchmaking()->CreateLobby(m_LobbyType, m_MaxLobbyMembers);
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false) &&
				((m_CallbackResults.m_eResult == k_EResultOK) ? true : false) &&
				((m_CallbackResults.m_ulSteamIDLobby > 0 ? true : false));
		}
	}
	else
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreCreateLobby::Finalize()
{
	LogSteamCoreVerbose("");
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

	if (bWasSuccessful)
	{
		if (FNamedOnlineSession* Session = SessionInt->GetNamedSession(m_SessionName))
		{
			ISteamMatchmaking* SteamMatchMakingPtr = SteamMatchmaking();
			check(SteamMatchMakingPtr);
			const FUniqueNetIdSteamRef LobbyId = FUniqueNetIdSteam::Create(m_CallbackResults.m_ulSteamIDLobby);

			const TSharedRef<FOnlineSessionInfoSteamCore> NewSessionInfo = MakeShared<FOnlineSessionInfoSteamCore>(ESteamSession::LobbySession, *LobbyId);
			NewSessionInfo->Init();
			NewSessionInfo->m_HostAddr = nullptr;
			TSharedRef<FInternetAddrSteamCore> SteamAddr = MakeShared<FInternetAddrSteamCore>(FUniqueNetIdSteam::Create(SteamUser()->GetSteamID()));
			SteamAddr->SetPort(Subsystem->GetGameServerGamePort());
			NewSessionInfo->m_SteamP2PAddr = SteamAddr;

			Session->SessionInfo = NewSessionInfo;
			Session->SessionState = EOnlineSessionState::Pending;

			FSteamSessionKeyValuePairs KeyValuePairs;
			GetLobbyKeyValuePairsFromSession(Session, KeyValuePairs);

			for (FSteamSessionKeyValuePairs::TConstIterator It(KeyValuePairs); It; ++It)
			{
				LogSteamCoreVerbose("Lobby Data (%s, %s)", *It.Key(), *It.Value());
				if (!SteamMatchMakingPtr->SetLobbyData(*LobbyId, TCHAR_TO_UTF8(*It.Key()), TCHAR_TO_UTF8(*It.Value())))
				{
					bWasSuccessful = false;
					break;
				}
			}

			if (!bWasSuccessful)
			{
				bWasSuccessful = false;
				SteamMatchMakingPtr->LeaveLobby(*LobbyId);
				SessionInt->RemoveNamedSession(m_SessionName);
				LogSteamCoreWarn("Failed to set lobby data for session %s, cleaning up.", *m_SessionName.ToString());
			}
			else
			{
				SessionInt->JoinedLobby(*LobbyId);
				SessionInt->RegisterLocalPlayers(Session);
				DumpNamedSession(Session);
			}
		}
	}
	else
	{
		SessionInt->RemoveNamedSession(m_SessionName);
	}
}

void FOnlineAsyncTaskSteamCoreCreateLobby::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		SessionInt->TriggerOnCreateSessionCompleteDelegates(m_SessionName, bWasSuccessful);
	}
}

FString FOnlineAsyncTaskSteamCoreJoinLobby::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreJoinLobby bWasSuccessful: %d Session: %s LobbyId: %s Result: %s"), WasSuccessful(), *m_SessionName.ToString(), *m_LobbyId->ToDebugString(), *SteamCore::SteamChatRoomEnterResponseString(static_cast<EChatRoomEnterResponse>(m_CallbackResults.m_EChatRoomEnterResponse)));
}

FString FOnlineAsyncTaskSteamCoreUpdateLobby::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreUpdateLobby bWasSuccessful: %d Session: %s"), WasSuccessful(), *m_SessionName.ToString());
}

void FOnlineAsyncTaskSteamCoreUpdateLobby::Tick()
{
	LogSteamCoreVerbose("");
	bWasSuccessful = false;

	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		FNamedOnlineSession* Session = SessionInt->GetNamedSession(m_SessionName);
		if (Session)
		{
			const TSharedPtr<FOnlineSessionInfoSteamCore> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoSteamCore>(Session->SessionInfo);
			if (SessionInfo.IsValid())
			{
				const bool bUsesPresence = Session->SessionSettings.bUsesPresence;
				if (bUsesPresence != m_NewSessionSettings.bUsesPresence)
				{
					LogSteamCoreWarn("Can't change presence settings on existing session %s, ignoring.", *m_SessionName.ToString());
				}

				FSteamSessionKeyValuePairs OldKeyValuePairs;
				GetLobbyKeyValuePairsFromSession(Session, OldKeyValuePairs);

				Session->SessionSettings = m_NewSessionSettings;
				Session->SessionSettings.bUsesPresence = bUsesPresence;

				if (m_bUpdateOnlineData)
				{
					ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();
					check(SteamMatchmakingPtr);

					const ELobbyType LobbyType = BuildLobbyTypeSteamCore(&Session->SessionSettings);
					if (SteamMatchmakingPtr->SetLobbyType(*SessionInfo->m_SessionId, LobbyType))
					{
						const int32 LobbyMemberCount = SteamMatchmakingPtr->GetNumLobbyMembers(*SessionInfo->m_SessionId);
						const int32 NumConnections = Session->SessionSettings.NumPrivateConnections + Session->SessionSettings.NumPublicConnections;

						if (SteamMatchmakingPtr->SetLobbyMemberLimit(*SessionInfo->m_SessionId, NumConnections))
						{
							const int32 MaxLobbyMembers = SteamMatchmakingPtr->GetLobbyMemberLimit(*SessionInfo->m_SessionId);
							const bool bLobbyJoinable = Session->SessionSettings.bAllowJoinInProgress && (MaxLobbyMembers != 0);
							if (SteamMatchmakingPtr->SetLobbyJoinable(*SessionInfo->m_SessionId, bLobbyJoinable))
							{
								bWasSuccessful = true;

								FSteamSessionKeyValuePairs KeyValuePairs;
								GetLobbyKeyValuePairsFromSession(Session, KeyValuePairs);

								for (FSteamSessionKeyValuePairs::TConstIterator It(OldKeyValuePairs); It; ++It)
								{
									LogSteamCoreVerbose("Removing Lobby Data (%s, %s)", *It.Key(), *It.Value());
									if (!SteamMatchmakingPtr->SetLobbyData(*SessionInfo->m_SessionId, TCHAR_TO_UTF8(*It.Key()), ""))
									{
										bWasSuccessful = false;
										break;
									}
								}

								if (bWasSuccessful)
								{
									for (FSteamSessionKeyValuePairs::TConstIterator It(KeyValuePairs); It; ++It)
									{
										LogSteamCoreVerbose("Updating Lobby Data (%s, %s)", *It.Key(), *It.Value());
										if (!SteamMatchmakingPtr->SetLobbyData(*SessionInfo->m_SessionId, TCHAR_TO_UTF8(*It.Key()), TCHAR_TO_UTF8(*It.Value())))
										{
											bWasSuccessful = false;
											break;
										}
									}
								}
							}
						}
					}
				}
				else
				{
					bWasSuccessful = true;
				}
			}
		}
	}

	bIsComplete = true;
}

void FOnlineAsyncTaskSteamCoreUpdateLobby::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		SessionInt->TriggerOnUpdateSessionCompleteDelegates(m_SessionName, bWasSuccessful);
	}
}

void FOnlineAsyncTaskSteamCoreJoinLobby::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	if (!m_bInit)
	{
		m_CallbackHandle = SteamMatchmaking()->JoinLobby(*m_LobbyId);
		m_bInit = true;
	}

	if (m_CallbackHandle != k_uAPICallInvalid)
	{
		bool bFailedCall = false;

		bIsComplete = SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall) ? true : false;
		if (bIsComplete)
		{
			bool bFailedResult;
			const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
			bWasSuccessful = (bSuccessCallResult ? true : false) &&
				(!bFailedCall ? true : false) &&
				(!bFailedResult ? true : false) &&
				((m_CallbackResults.m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess) ? true : false) &&
				(CSteamID(m_CallbackResults.m_ulSteamIDLobby) == *m_LobbyId ? true : false);
		}
	}
	else
	{
		bIsComplete = true;
		bWasSuccessful = false;
	}
}

void FOnlineAsyncTaskSteamCoreJoinLobby::Finalize()
{
	LogSteamCoreVerbose("");
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (SessionInt.IsValid())
	{
		if (bWasSuccessful)
		{
			if (FNamedOnlineSession* Session = SessionInt->GetNamedSession(m_SessionName))
			{
				Session->SessionState = EOnlineSessionState::Pending;
				SessionInt->JoinedLobby(*m_LobbyId);
				SessionInt->RegisterLocalPlayers(Session);
			}
			else
			{
				LogSteamCoreWarn("Session %s not found when trying to join", *m_SessionName.ToString());
			}
		}
	}

	if (!bWasSuccessful)
	{
		SessionInt->RemoveNamedSession(m_SessionName);
	}
}

void FOnlineAsyncTaskSteamCoreJoinLobby::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		SessionInt->TriggerOnJoinSessionCompleteDelegates(m_SessionName, bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	}
}

FString FOnlineAsyncTaskSteamCoreLeaveLobby::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreLeaveLobby bWasSuccessful: %d SessionName: %s LobbyId: %s"), WasSuccessful(), *m_SessionName.ToString(), *m_LobbyId->ToDebugString());
}

void FOnlineAsyncTaskSteamCoreLeaveLobby::Tick()
{
	LogSteamCoreVerbose("");
	SteamMatchmaking()->LeaveLobby(*m_LobbyId);
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (SessionInt.IsValid())
	{
		SessionInt->LeftLobby(*m_LobbyId);
	}

	bIsComplete = true;
	bWasSuccessful = true;
}

void FOnlineAsyncTaskSteamCoreFindLobbiesBase::CreateQuery()
{
	LogSteamCoreVerbose("");
	check(m_SteamMatchmakingPtr);

	if (m_SearchSettings->MaxSearchResults > 0)
	{
		m_SteamMatchmakingPtr->AddRequestLobbyListResultCountFilter(m_SearchSettings->MaxSearchResults);
	}

	const FOnlineSessionSearchSteam* OnlineSessionSearchSteam = static_cast<FOnlineSessionSearchSteam*>(m_SearchSettings.Get());
	
	int32 DistanceFilter = static_cast<int32>(OnlineSessionSearchSteam->m_LobbyDistanceFilter);

	LogSteamCoreVerbose("FOnlineAsyncTaskSteamCoreFindLobbiesBase::CreateQuery using Distance Filter: %i", DistanceFilter);
	
	m_SteamMatchmakingPtr->AddRequestLobbyListDistanceFilter(static_cast<ELobbyDistanceFilter>(DistanceFilter));

	for (FSearchParams::TConstIterator It(m_SearchSettings->QuerySettings.SearchParams); It; ++It)
	{
		const FName Key = It.Key();
		const FOnlineSessionSearchParam& SearchParam = It.Value();

		if (Key == SEARCH_DEDICATED_ONLY || Key == SETTING_MAPNAME || Key == SEARCH_EMPTY_SERVERS_ONLY || Key == SEARCH_SECURE_SERVERS_ONLY || Key == SEARCH_PRESENCE)
		{
			continue;
		}

		FString KeyStr;
		if (SessionKeyToSteamKey(Key, SearchParam.Data, KeyStr))
		{
			if (SearchParam.ComparisonOp == EOnlineComparisonOp::Near)
			{
				switch (SearchParam.Data.GetType())
				{
				case EOnlineKeyValuePairDataType::Int32:
					{
						int32 Value;
						SearchParam.Data.GetValue(Value);
						m_SteamMatchmakingPtr->AddRequestLobbyListNearValueFilter(TCHAR_TO_UTF8(*KeyStr), Value);
						break;
					}
				default:
					LogSteamCoreWarn("Unable to set search parameter %s", *SearchParam.ToString());
					break;
				}
			}
			else
			{
				switch (SearchParam.Data.GetType())
				{
				case EOnlineKeyValuePairDataType::Int32:
					{
						int32 Value;
						SearchParam.Data.GetValue(Value);
						m_SteamMatchmakingPtr->AddRequestLobbyListNumericalFilter(TCHAR_TO_UTF8(*KeyStr), Value, ToSteamLobbyCompareOp(SearchParam.ComparisonOp));
						break;
					}
				case EOnlineKeyValuePairDataType::Float:
					{
						m_SteamMatchmakingPtr->AddRequestLobbyListStringFilter(TCHAR_TO_UTF8(*KeyStr), TCHAR_TO_UTF8(*SearchParam.Data.ToString()), ToSteamLobbyCompareOp(SearchParam.ComparisonOp));
						break;
					}
				case EOnlineKeyValuePairDataType::String:
					{
						FString Value;
						SearchParam.Data.GetValue(Value);

						if (!Value.IsEmpty())
						{
							m_SteamMatchmakingPtr->AddRequestLobbyListStringFilter(TCHAR_TO_UTF8(*KeyStr), TCHAR_TO_UTF8(*Value), ToSteamLobbyCompareOp(SearchParam.ComparisonOp));
						}
						else
						{
							LogSteamCoreWarn("Empty search parameter %s: %s", *Key.ToString(), *SearchParam.ToString());
						}

						break;
					}
				default:
					LogSteamCoreWarn("Unable to set search parameter %s: %s", *Key.ToString(), *SearchParam.ToString());
					break;
				}
			}
		}
		else
		{
			LogSteamCoreWarn("Unsupported search setting %s %s of type %s", *Key.ToString(), *SearchParam.ToString(), EOnlineComparisonOp::ToString(SearchParam.ComparisonOp));
		}
	}
}

void FOnlineAsyncTaskSteamCoreFindLobbiesBase::ParseSearchResult(const FUniqueNetIdSteam& LobbyId) const
{
	LogSteamCoreVerbose("");
	FOnlineSessionSearchResult* NewSearchResult = new(m_SearchSettings->SearchResults) FOnlineSessionSearchResult();
	if (!SteamCore::FillSessionFromLobbyData(Subsystem, LobbyId, NewSearchResult->Session, NewSearchResult))
	{
		LogSteamCoreWarn("Unable to parse search result for lobby '%s'", *LobbyId.ToDebugString());
		m_SearchSettings->SearchResults.RemoveAtSwap(m_SearchSettings->SearchResults.Num() - 1);
	}
}

void FOnlineAsyncTaskSteamCoreFindLobbiesBase::Tick()
{
	LogSteamCoreVerbose("");
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	check(SteamUtilsPtr);

	switch (m_FindLobbiesState)
	{
	case EFindLobbiesState::Init:
		{
			if (ISocketSubsystem::Get()->HasNetworkDevice())
			{
				if (SteamUser()->BLoggedOn())
				{
					LogSteamCoreVerbose("Starting search for Internet games...");

					CreateQuery();
					m_CallbackHandle = m_SteamMatchmakingPtr->RequestLobbyList();
				}
				else
				{
					LogSteamCoreWarn("You must be logged in to an online profile to search for internet games");
				}
			}
			else
			{
				LogSteamCoreWarn("Can't search for an internet game without a network connection");
			}

			if (m_CallbackHandle == k_uAPICallInvalid)
			{
				bWasSuccessful = false;
				m_FindLobbiesState = EFindLobbiesState::Finished;
			}
			else
			{
				m_FindLobbiesState = EFindLobbiesState::RequestLobbyList;
			}
			break;
		}
	case EFindLobbiesState::RequestLobbyList:
		{
			bool bFailedCall = false;
			if (SteamUtilsPtr->IsAPICallCompleted(m_CallbackHandle, &bFailedCall))
			{
				bool bFailedResult;
				const bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(m_CallbackHandle, &m_CallbackResults, sizeof(m_CallbackResults), m_CallbackResults.k_iCallback, &bFailedResult);
				bWasSuccessful = bSuccessCallResult && !bFailedCall && !bFailedResult;
				if (bWasSuccessful)
				{
					const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
					check(SessionInt.IsValid());

					const int32 NumLobbies = static_cast<int32>(m_CallbackResults.m_nLobbiesMatching);
					for (int32 LobbyIdx = 0; LobbyIdx < NumLobbies; LobbyIdx++)
					{
						FUniqueNetIdSteamRef LobbyID = FUniqueNetIdSteam::Create(m_SteamMatchmakingPtr->GetLobbyByIndex(LobbyIdx));
						if (!SessionInt->IsMemberOfLobby(*LobbyID))
						{
							m_LobbyIDs.Add(**LobbyID);
						}
					}
					m_FindLobbiesState = EFindLobbiesState::RequestLobbyData;
				}
				else
				{
					m_FindLobbiesState = EFindLobbiesState::Finished;
				}
			}
			break;
		}
	case EFindLobbiesState::RequestLobbyData:
		{
			bWasSuccessful = true;
			for (const CSteamID LobbyId : m_LobbyIDs)
			{
				if (!m_SteamMatchmakingPtr->RequestLobbyData(LobbyId))
				{
					bWasSuccessful = false;
					m_FindLobbiesState = EFindLobbiesState::Finished;
					break;
				}
			}

			if (bWasSuccessful)
			{
				m_FindLobbiesState = EFindLobbiesState::WaitForRequestLobbyData;
			}

			break;
		}
	case EFindLobbiesState::WaitForRequestLobbyData:
		{
			const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

			if (m_LobbyIDs.Num() == SessionInt->m_PendingSearchLobbyIds.Num())
			{
				m_FindLobbiesState = EFindLobbiesState::Finished;
			}

			else if (GetElapsedTime() >= ASYNC_TASK_TIMEOUT)
			{
				bWasSuccessful = false;
				m_FindLobbiesState = EFindLobbiesState::Finished;
			}
			break;
		}
	case EFindLobbiesState::Finished:
		{
			bIsComplete = true;
			break;
		}
	default:
		{
			LogSteamCoreWarn("Unexpected state %d reached in FOnlineAsyncTaskSteamCoreFindLobbiesBase::Tick, ending task.", static_cast<int32>(m_FindLobbiesState));
			bWasSuccessful = false;
			m_FindLobbiesState = EFindLobbiesState::Finished;
			break;
		}
	}
}

void FOnlineAsyncTaskSteamCoreFindLobbiesBase::Finalize()
{
	LogSteamCoreVerbose("");
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());

	LogSteamCoreVerbose("Found %d lobbies, finalizing the search", SessionInt->m_PendingSearchLobbyIds.Num());

	if (bWasSuccessful)
	{
		for (int32 LobbyIdx = 0; LobbyIdx < SessionInt->m_PendingSearchLobbyIds.Num(); LobbyIdx++)
		{
			const FUniqueNetIdSteam& LobbyId = *SessionInt->m_PendingSearchLobbyIds[LobbyIdx];
			LogSteamCoreVerbose("Search result %d: LobbyId=%s, LobbyId.IsValid()=%s, CSteamID(LobbyId).IsLobby()=%s", LobbyIdx, *LobbyId.ToDebugString(), LobbyId.IsValid() ? TEXT("true") : TEXT("false"), (*LobbyId).IsLobby() ? TEXT("true") : TEXT("false"));

			if (LobbyId.IsValid() && (*LobbyId).IsLobby())
			{
				ParseSearchResult(LobbyId);
			}
			else
			{
				LogSteamCoreWarn("Lobby %d is invalid (or not a lobby), skipping.", LobbyIdx);
			}
		}

		if (m_SearchSettings->SearchResults.Num() > 0)
		{
			m_SearchSettings->SortSearchResults();
		}
	}

	m_SearchSettings->SearchState = bWasSuccessful ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;
	if (SessionInt->m_CurrentSessionSearch.IsValid() && m_SearchSettings == SessionInt->m_CurrentSessionSearch)
	{
		SessionInt->m_CurrentSessionSearch = NULL;
	}

	SessionInt->m_PendingSearchLobbyIds.Empty();
}

FString FOnlineAsyncTaskSteamCoreFindLobbies::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindLobbiesForFindSessions bWasSuccessful: %d NumResults: %d"), WasSuccessful(), m_CallbackResults.m_nLobbiesMatching);
}

void FOnlineAsyncTaskSteamCoreFindLobbies::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	const IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
	if (SessionInt.IsValid())
	{
		SessionInt->TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);
	}
}

FString FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession bWasSuccessful: %d Lobby ID: %llu"), WasSuccessful(), m_LobbyIDs[0].ConvertToUint64());
}

void FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	if (bWasSuccessful && m_SearchSettings->SearchResults.Num() > 0)
	{
		if (m_FriendId.Get().IsValid())
		{
			m_OnFindLobbyCompleteWithNetIdDelegate.Broadcast(bWasSuccessful, m_LocalUserNum, m_FriendId, m_SearchSettings->SearchResults[0]);
		}
		else
		{
			m_OnFindLobbyCompleteWithNetIdDelegate.Broadcast(bWasSuccessful, m_LocalUserNum, FUniqueNetIdSteam::Create(SteamUser()->GetSteamID()), m_SearchSettings->SearchResults[0]);
		}
	}
	else
	{
		const FOnlineSessionSearchResult EmptyResult;
		m_OnFindLobbyCompleteWithNetIdDelegate.Broadcast(bWasSuccessful, m_LocalUserNum, FUniqueNetIdSteam::Create(SteamUser()->GetSteamID()), EmptyResult);
	}
}

FString FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession bWasSuccessful: %d Lobby ID: %llu"), WasSuccessful(), m_LobbyIDs[0].ConvertToUint64());
}

void FOnlineAsyncTaskSteamCoreFindLobbiesForFriendSession::TriggerDelegates()
{
	LogSteamCoreVerbose("");
	if (bWasSuccessful && m_SearchSettings->SearchResults.Num() > 0)
	{
		m_OnFindFriendSessionCompleteDelegate.Broadcast(m_LocalUserNum, bWasSuccessful, m_SearchSettings->SearchResults);
	}
	else
	{
		const TArray<FOnlineSessionSearchResult> EmptyResult;
		m_OnFindFriendSessionCompleteDelegate.Broadcast(m_LocalUserNum, bWasSuccessful, EmptyResult);
	}
}

FString FOnlineAsyncEventSteamCoreLobbyInviteAccepted::ToString() const
{
	return FString::Printf(TEXT("FOnlineAsyncEventSteamLobbyInviteAccepted LobbyId: %s Friend: %s"), *m_LobbyId->ToDebugString(), *m_FriendId->ToDebugString());
}

void FOnlineAsyncEventSteamCoreLobbyInviteAccepted::Finalize()
{
	LogSteamCoreVerbose("");
	const FOnlineSessionSteamCorePtr SessionInt = StaticCastSharedPtr<FOnlineSessionSteamCore>(Subsystem->GetSessionInterface());
	if (SessionInt.IsValid() && !SessionInt->m_CurrentSessionSearch.IsValid())
	{
		SessionInt->m_CurrentSessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionInt->m_CurrentSessionSearch->SearchState = EOnlineAsyncTaskState::InProgress;

		FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession* NewTask = new FOnlineAsyncTaskSteamCoreFindLobbiesForInviteSession(Subsystem, m_FriendId, *m_LobbyId, SessionInt->m_CurrentSessionSearch, m_LocalUserNum, SessionInt->OnSessionUserInviteAcceptedDelegates);
		Subsystem->QueueAsyncTask(NewTask);
	}
	else
	{
		LogSteamCoreWarn("Invalid session or search already in progress when accepting invite.  Ignoring invite request.");
	}
}
#endif