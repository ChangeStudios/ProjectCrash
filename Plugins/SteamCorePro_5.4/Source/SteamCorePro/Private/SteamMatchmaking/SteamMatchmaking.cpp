/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamMatchmaking
*/

#include "SteamMatchmaking/SteamMatchmaking.h"
#include "SteamMatchmaking/SteamMatchmakingAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProMatchmaking::USteamProMatchmaking()
{
#if WITH_STEAMCORE
	OnFavoritesListAccountsUpdatedCallback.Register(this, &USteamProMatchmaking::OnFavoritesListAccountsUpdated);
	OnFavoritesListChangedCallback.Register(this, &USteamProMatchmaking::OnFavoritesListChanged);
	OnLobbyChatMsgCallback.Register(this, &USteamProMatchmaking::OnLobbyChatMsg);
	OnLobbyChatUpdateCallback.Register(this, &USteamProMatchmaking::OnLobbyChatUpdate);
	OnLobbyDataUpdateCallback.Register(this, &USteamProMatchmaking::OnLobbyDataUpdate);
	OnLobbyEnterCallback.Register(this, &USteamProMatchmaking::OnLobbyEnter);
	OnLobbyGameCreatedCallback.Register(this, &USteamProMatchmaking::OnLobbyGameCreated);
	OnLobbyInviteCallback.Register(this, &USteamProMatchmaking::OnLobbyInvite);
	OnLobbyKickedCallback.Register(this, &USteamProMatchmaking::OnLobbyKicked);

	if (IsRunningDedicatedServer())
	{
		OnFavoritesListAccountsUpdatedCallback.SetGameserverFlag();
		OnFavoritesListChangedCallback.SetGameserverFlag();
		OnLobbyChatMsgCallback.SetGameserverFlag();
		OnLobbyChatUpdateCallback.SetGameserverFlag();
		OnLobbyDataUpdateCallback.SetGameserverFlag();
		OnLobbyEnterCallback.SetGameserverFlag();
		OnLobbyGameCreatedCallback.SetGameserverFlag();
		OnLobbyInviteCallback.SetGameserverFlag();
		OnLobbyKickedCallback.SetGameserverFlag();
	}
#endif
}

USteamProMatchmaking::~USteamProMatchmaking()
{
#if WITH_STEAMCORE
	OnFavoritesListAccountsUpdatedCallback.Unregister();
	OnFavoritesListChangedCallback.Unregister();
	OnLobbyChatMsgCallback.Unregister();
	OnLobbyChatUpdateCallback.Unregister();
	OnLobbyDataUpdateCallback.Unregister();
	OnLobbyEnterCallback.Unregister();
	OnLobbyGameCreatedCallback.Unregister();
	OnLobbyInviteCallback.Unregister();
	OnLobbyKickedCallback.Unregister();
#endif
}

USteamProMatchmaking* USteamProMatchmaking::GetSteamMatchmaking()
{
#if WITH_STEAMCORE
	return SteamMatchmaking() ? ThisClass::StaticClass()->GetDefaultObject<USteamProMatchmaking>() : nullptr;
#else
	return nullptr;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //


int32 USteamProMatchmaking::AddFavoriteGame(int32 AppID, FString IP, int32 ConnectionPort, int32 QueryPort, TArray<ESteamFavoriteFlags> Flags, int32 TimeLastPlayedOnServer)
{
	LogSteamCoreVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		int Data = 0;

		for (int i = 0; i < Flags.Num(); i++)
		{
			Data |= static_cast<int>(Flags[i]);
		}

		FIPv4Address IpAddress;
		FIPv4Address::Parse(IP, IpAddress);

		Result = SteamMatchmaking()->AddFavoriteGame(AppID, IpAddress.Value, ConnectionPort, QueryPort, Data, TimeLastPlayedOnServer);
	}
#endif

	return Result;
}

void USteamProMatchmaking::AddRequestLobbyListCompatibleMembersFilter(FSteamID SteamIDLobby)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListCompatibleMembersFilter(SteamIDLobby);
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListDistanceFilter(ESteamLobbyDistanceFilter LobbyDistanceFilter)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListDistanceFilter(static_cast<ELobbyDistanceFilter>(LobbyDistanceFilter));
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListFilterSlotsAvailable(int32 SlotsAvailable)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(SlotsAvailable);
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListNearValueFilter(FString KeyToMatch, int32 ValueToBeCloseTo)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListNearValueFilter(TCHAR_TO_UTF8(*KeyToMatch), ValueToBeCloseTo);
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListNumericalFilter(FString KeyToMatch, int32 ValueToMatch, ESteamLobbyComparison ComparisonType)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListNumericalFilter(TCHAR_TO_UTF8(*KeyToMatch), ValueToMatch, static_cast<ELobbyComparison>((static_cast<uint8>(ComparisonType) - 2)));
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListResultCountFilter(int32 MaxResults)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddRequestLobbyListResultCountFilter(MaxResults);
	}
#endif
}

void USteamProMatchmaking::AddRequestLobbyListStringFilter(FString KeyToMatch, FString ValueToMatch, ESteamLobbyComparison ComparisonType)
{
	LogSteamCoreVerbose("");

	if (KeyToMatch.Len() == 0 || ValueToMatch.Len() == 0)
	{
		return;
	}

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		const FTCHARToUTF8 KeyAnsi(*KeyToMatch); 
		const FTCHARToUTF8 ValueAnsi(*ValueToMatch);

		SteamMatchmaking()->AddRequestLobbyListStringFilter(KeyAnsi.Get(), ValueAnsi.Get(), static_cast<ELobbyComparison>((static_cast<uint8>(ComparisonType) - 2)));
	}
#endif
}

void USteamProMatchmaking::CreateLobby(const FOnCreateLobby& Callback, ESteamLobbyType LobbyType, int32 MaxMembers)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby* Task = new FOnlineAsyncTaskSteamCoreProMatchmakingCreateLobby(Callback, static_cast<ELobbyType>(LobbyType), MaxMembers);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProMatchmaking::DeleteLobbyData(FSteamID SteamIDLobby, FString Key)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->DeleteLobbyData(SteamIDLobby, TCHAR_TO_UTF8(*Key));
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::GetFavoriteGame(int32 Game, int32& OutAppID, FString& OutIp, int32& OutConnectionPort, int32& OutQueryPort, TArray<ESteamFavoriteFlags>& OutFlags, int32& OutTimeLastPlayedOnServer)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

	OutFlags.Empty();
	OutIp.Empty();

	uint32 IpAddress = 0;
	uint32 Flags = 0;

	uint16 ConnectionPort = 0;
	uint16 QueryPort = 0;
	uint32 TimeLastPlayedOnServer = 0;
	uint32 AppId = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->GetFavoriteGame(Game, &AppId, &IpAddress, &ConnectionPort, &QueryPort, &Flags, &TimeLastPlayedOnServer);

		if (bResult)
		{
			FIPv4Address InternalIP(IpAddress);
			OutIp = InternalIP.ToString();

			for (uint8 i = 0; i < 31; i++)
			{
				if (Flags & 1 << i)
				{
					OutFlags.Add(static_cast<ESteamFavoriteFlags>(1 << i));
				}
			}
		}
	}
#endif

	OutConnectionPort = ConnectionPort;
	OutQueryPort = QueryPort;
	OutTimeLastPlayedOnServer = TimeLastPlayedOnServer;
	OutAppID = AppId;

	return bResult;
}

int32 USteamProMatchmaking::GetFavoriteGameCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetFavoriteGameCount();
	}
#endif

	return Result;
}

FSteamID USteamProMatchmaking::GetLobbyByIndex(int32 Lobby)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetLobbyByIndex(Lobby);
	}
#endif

	return Result;
}

int32 USteamProMatchmaking::GetLobbyChatEntry(FSteamID SteamIDLobby, int32 MessageID, FSteamID& OutSteamIDUser, FString& OutMessage, ESteamChatEntryType& OutChatEntryType)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutMessage.Empty();
	OutSteamIDUser = FSteamID();
	OutChatEntryType = ESteamChatEntryType::Invalid;
	
#if WITH_STEAMCORE
	CSteamID SteamIdUser;
	EChatEntryType ChatEntryType = k_EChatEntryTypeInvalid;

	if (SteamMatchmaking())
	{
		TArray<uint8> DataArray;
		DataArray.SetNum(1024);

		Result = SteamMatchmaking()->GetLobbyChatEntry(SteamIDLobby, MessageID, &SteamIdUser, DataArray.GetData(), DataArray.Num(), &ChatEntryType);
		DataArray.SetNum(Result);

		if (Result > 0)
		{
			FMemoryReader Reader(DataArray);
			Reader << OutMessage;
			Reader.Close();
		}
	}

	OutChatEntryType = static_cast<ESteamChatEntryType>(ChatEntryType);
	OutSteamIDUser = SteamIdUser;
#endif

	return Result;
}

FString USteamProMatchmaking::GetLobbyData(FSteamID SteamIDLobby, FString Key)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyData(SteamIDLobby, TCHAR_TO_UTF8(*Key)));
	}
#endif

	return Result;
}

bool USteamProMatchmaking::GetLobbyDataByIndex(FSteamID SteamIDLobby, int32 LobbyData, FString& OutKey, FString& OutValue)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutKey.Empty();
	OutValue.Empty();

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		TArray<char> Keys;
		TArray<char> Value;
		Keys.SetNum(k_nMaxLobbyKeyLength);
		Value.SetNum(k_cubChatMetadataMax);

		bResult = SteamMatchmaking()->GetLobbyDataByIndex(SteamIDLobby, LobbyData, Keys.GetData(), k_nMaxLobbyKeyLength, Value.GetData(), k_cubChatMetadataMax);

		if (bResult)
		{
			TCHAR KeyChar[k_nMaxLobbyKeyLength];
			FCString::Strncpy(KeyChar, UTF8_TO_TCHAR(Keys.GetData()), k_nMaxLobbyKeyLength);
			TCHAR ValueChar[k_cubChatMetadataMax];
			FCString::Strncpy(ValueChar, UTF8_TO_TCHAR(Value.GetData()), k_cubChatMetadataMax);

			OutKey = KeyChar;
			OutValue = ValueChar;
		}
	}
#endif

	return bResult;
}

int32 USteamProMatchmaking::GetLobbyDataCount(FSteamID SteamIDLobby)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetLobbyDataCount(SteamIDLobby);
	}
#endif

	return Result;
}

bool USteamProMatchmaking::GetLobbyGameServer(FSteamID SteamIDLobby, FString& OutGameServerIP, int32& OutGameServerPort, FSteamID& OutSteamIDGameServer)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutGameServerIP.Empty();

	uint32 GameServerIP = 0;
	uint16 GameServerPort = 0;
	OutGameServerPort = 0;
	OutSteamIDGameServer = FSteamID();
	
#if WITH_STEAMCORE
	CSteamID SteamIDGameServer;

	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->GetLobbyGameServer(SteamIDLobby, &GameServerIP, &GameServerPort, &SteamIDGameServer);

		if (bResult)
		{
			OutGameServerIP = FIPv4Address(GameServerIP).ToString();
		}
	}
	OutSteamIDGameServer = SteamIDGameServer;
#endif

	OutGameServerPort = GameServerPort;

	return bResult;
}

FSteamID USteamProMatchmaking::GetLobbyMemberByIndex(FSteamID SteamIDLobby, int32 Member)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetLobbyMemberByIndex(SteamIDLobby, Member);
	}
#endif

	return Result;
}

FString USteamProMatchmaking::GetLobbyMemberData(FSteamID SteamIDLobby, FSteamID SteamIDUser, FString Key)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = UTF8_TO_TCHAR(SteamMatchmaking()->GetLobbyMemberData(SteamIDLobby, SteamIDUser, TCHAR_TO_UTF8(*Key)));
	}
#endif

	return Result;
}

int32 USteamProMatchmaking::GetLobbyMemberLimit(FSteamID SteamIDLobby)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetLobbyMemberLimit(SteamIDLobby);
	}
#endif

	return Result;
}

FSteamID USteamProMatchmaking::GetLobbyOwner(FSteamID SteamIDLobby)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetLobbyOwner(SteamIDLobby);
	}
#endif

	return Result;
}

int32 USteamProMatchmaking::GetNumLobbyMembers(FSteamID SteamIDLobby)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		Result = SteamMatchmaking()->GetNumLobbyMembers(SteamIDLobby);
	}
#endif

	return Result;
}

bool USteamProMatchmaking::InviteUserToLobby(FSteamID SteamIDLobby, FSteamID SteamIDInvitee)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->InviteUserToLobby(SteamIDLobby, SteamIDInvitee);
	}
#endif

	return bResult;
}

void USteamProMatchmaking::JoinLobby(const FOnJoinLobby& Callback, FSteamID SteamIDLobby)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		FOnlineAsyncTaskSteamCoreProJoinLobby* Task = new FOnlineAsyncTaskSteamCoreProJoinLobby(Callback, SteamIDLobby);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProMatchmaking::LeaveLobby(FSteamID SteamIDLobby)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->LeaveLobby(SteamIDLobby);
	}
#endif
}

bool USteamProMatchmaking::RemoveFavoriteGame(int32 AppID, FString IP, int32 ConnectionPort, int32 QueryPort, TArray<ESteamFavoriteFlags> Flags)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		int Data = 0;

		for (int i = 0; i < Flags.Num(); i++)
		{
			Data |= static_cast<int>(Flags[i]);
		}

		FIPv4Address IpAddress;
		FIPv4Address::Parse(IP, IpAddress);

		bResult = SteamMatchmaking()->RemoveFavoriteGame(AppID, IpAddress.Value, ConnectionPort, QueryPort, Data);
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::RequestLobbyData(FSteamID SteamIDLobby)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->RequestLobbyData(SteamIDLobby);
	}
#endif

	return bResult;
}

void USteamProMatchmaking::RequestLobbyList(const FOnRequestLobbyList& Callback)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		FOnlineAsyncTaskSteamCoreProRequestLobbyList* Task = new FOnlineAsyncTaskSteamCoreProRequestLobbyList(Callback);
		QueueAsyncTask(Task);
	}
#endif
}

bool USteamProMatchmaking::SendLobbyChatMsg(FSteamID SteamIDLobby, FString Message)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		TArray<uint8> DataArray;
		DataArray.SetNum(Message.Len());
		FMemoryWriter Writer(DataArray);
		Writer << Message;
		Writer.Close();

		bResult = SteamMatchmaking()->SendLobbyChatMsg(SteamIDLobby, DataArray.GetData(), DataArray.Num());
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::SetLinkedLobby(FSteamID SteamIDLobby, FSteamID SteamIDLobbyDependent)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->SetLinkedLobby(SteamIDLobby, SteamIDLobbyDependent);
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::SetLobbyData(FSteamID SteamIDLobby, FString Key, FString Value)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

	if (Key.Len() == 0 || Value.Len() == 0)
	{
		return bResult;
	}

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		const FTCHARToUTF8 ConvertedKey(*Key);
		const FTCHARToUTF8 ConvertedValue(*Value);

		bResult = SteamMatchmaking()->SetLobbyData(SteamIDLobby, ConvertedKey.Get(), ConvertedValue.Get());
	}
#endif

	return bResult;
}

void USteamProMatchmaking::SetLobbyGameServer(FSteamID SteamIDLobby, FString GameServerIP, int32 GameServerPort, FSteamID SteamIDGameServer)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		FIPv4Address IP;
		FIPv4Address::Parse(GameServerIP, IP);

		SteamMatchmaking()->SetLobbyGameServer(SteamIDLobby, IP.Value, GameServerPort, SteamIDGameServer);
	}
#endif
}

bool USteamProMatchmaking::SetLobbyJoinable(FSteamID SteamIDLobby, bool bLobbyJoinable)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->SetLobbyJoinable(SteamIDLobby, bLobbyJoinable);
	}
#endif

	return bResult;
}

void USteamProMatchmaking::SetLobbyMemberData(FSteamID SteamIDLobby, FString Key, FString Value)
{
	LogSteamCoreVerbose("");

	if (Key.Len() == 0 || Value.Len() == 0)
	{
		return;
	}

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		const FTCHARToUTF8 ConvertedKey(*Key);
		const FTCHARToUTF8 ConvertedValue(*Value);

		SteamMatchmaking()->SetLobbyMemberData(SteamIDLobby, ConvertedKey.Get(), ConvertedValue.Get());
	}
#endif
}

bool USteamProMatchmaking::SetLobbyMemberLimit(FSteamID SteamIDLobby, int32 MaxMembers)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->SetLobbyMemberLimit(SteamIDLobby, MaxMembers);
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::SetLobbyOwner(FSteamID SteamIDLobby, FSteamID SteamIDNewOwner)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->SetLobbyOwner(SteamIDLobby, SteamIDNewOwner);
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::SetLobbyType(FSteamID SteamIDLobby, ESteamLobbyType LobbyType)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		bResult = SteamMatchmaking()->SetLobbyType(SteamIDLobby, static_cast<ELobbyType>(LobbyType));
	}
#endif

	return bResult;
}

bool USteamProMatchmaking::KickLobbyMember(FSteamID LobbyId, FSteamID PlayerToKick)
{
#if WITH_STEAMCORE
	if (SteamMatchmaking())
	{
		FSteamID LobbyOwner = GetLobbyOwner(LobbyId);
		if (LobbyOwner.IsValid())
		{
			if (LobbyOwner.GetValue() == SteamUser()->GetSteamID().ConvertToUint64())
			{
				const FString TheMessage = FString::Printf(TEXT("%llu KICKEDFROMLOBBY %llu"), static_cast<uint64_t>(PlayerToKick), static_cast<uint64_t>(LobbyId));
				SendLobbyChatMsg(LobbyId, TheMessage);

				return true;
			}
		}
	}
#endif

	return false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProMatchmaking::OnFavoritesListAccountsUpdated(FavoritesListAccountsUpdated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		FavoritesListAccountsUpdated.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnFavoritesListChanged(FavoritesListChanged_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		FavoritesListChanged.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyChatMsg(LobbyChatMsg_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyChatMsg.Broadcast(Data);

		FString TheMessage;
		ESteamChatEntryType Type;
		FSteamID SteamIduser = Data.m_ulSteamIDUser;
		GetLobbyChatEntry(Data.m_ulSteamIDLobby, Data.m_iChatID, SteamIduser, TheMessage, Type);

		if (TheMessage.Contains("KICKEDFROMLOBBY"))
		{
			LobbyKicked_t LobbyKickedData;
			LobbyKickedData.m_ulSteamIDLobby = Data.m_ulSteamIDLobby;
			LobbyKickedData.m_ulSteamIDAdmin = Data.m_ulSteamIDLobby;
			OnLobbyKicked(&LobbyKickedData);
		}
	});
}

void USteamProMatchmaking::OnLobbyChatUpdate(LobbyChatUpdate_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyChatUpdate.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyDataUpdate(LobbyDataUpdate_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyDataUpdate.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyEnter(LobbyEnter_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyEnter.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyGameCreated(LobbyGameCreated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyGameCreated.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyInvite(LobbyInvite_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyInvite.Broadcast(Data);
	});
}

void USteamProMatchmaking::OnLobbyKicked(LobbyKicked_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LobbyKicked.Broadcast(Data);
	});
}
#endif