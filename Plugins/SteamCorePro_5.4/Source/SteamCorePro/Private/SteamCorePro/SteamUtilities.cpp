/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official SteamCorePro Documentation: https://eeldev.com
*/

#include "SteamCorePro/SteamUtilities.h"

#include "LatentActions.h"
#include "OnlinePingInterfaceSteamCore.h"
#include "OnlineSubsystemSteamCore.h"
#include "SteamMatchmakingServers/SteamMatchmakingServersTypes.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "SteamCoreProPluginPrivatePCH.h"
#include "Online.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Net/OnlineEngineInterface.h"

TArray<FOnSteamMessage> USteamUtilities::s_SteamMessageListeners;
FDelegateHandle USteamUtilities::s_SessionInviteAcceptedDelegateHandle;
FDelegateHandle USteamUtilities::s_SessionInviteReceivedDelegateHandle;

FReadFriendListLatent::FReadFriendListLatent(const FLatentActionInfo& LatentInfo, EFriendListType FriendListType)
	: m_ExecutionFunction(LatentInfo.ExecutionFunction)
	, m_OutputLink(LatentInfo.Linkage)
	, m_CallbackTarget(LatentInfo.CallbackTarget)
	, bUpdatedFriendsList(false)
	, bRequestedFriendsList(false)
	, m_FriendListType(FriendListType)
{
}

FReadFriendListLatent::~FReadFriendListLatent()
{
}

void FReadFriendListLatent::UpdateOperation(FLatentResponse& Response)
{
#if WITH_STEAMCORE
	if (!bRequestedFriendsList)
	{
		bRequestedFriendsList = true;

		const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
		if (SteamSubsystem && SteamSubsystem->GetFriendsInterface())
		{
			SteamSubsystem->GetFriendsInterface()->ReadFriendsList(0, EFriendsLists::ToString((static_cast<EFriendsLists::Type>(m_FriendListType))), FOnReadFriendsListComplete::CreateLambda([this](int32, bool, const FString&, const FString&)
			{
				bUpdatedFriendsList = true;
			}));
		}
	}
#else
	bUpdatedFriendsList = true;
	bRequestedFriendsList = true;
#endif

	if (bUpdatedFriendsList && bRequestedFriendsList)
	{
		Response.FinishAndTriggerIf(true, m_ExecutionFunction, m_OutputLink, m_CallbackTarget);
	}
}

#if WITH_EDITOR
FString FReadFriendListLatent::GetDescription() const
{
	return FString::Printf(TEXT("FReadFriendListLatent::GetDescription"));
}
#endif


USteamCoreProVoice::USteamCoreProVoice(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Duration = INDEFINITELY_LOOPING_DURATION;
	NumChannels = 1;
	bLooping = false;
	bCanProcessAsync = true;
}

void USteamCoreProVoice::AddAudioBuffer(const TArray<uint8>& Buffer)
{
	QueueAudio(Buffer.GetData(), Buffer.Num());
}

void USteamCoreProVoice::DestroySteamCoreProVoice(USteamCoreProVoice* OBJ)
{
	LogSteamCoreVerbose("");

	if (OBJ)
	{
		OBJ->RemoveFromRoot();
	}
}

USteamCoreProVoice* USteamCoreProVoice::ConstructSteamCoreProVoice(int32 AudioSampleRate)
{
	LogSteamCoreVerbose("");

	check(AudioSampleRate > 0);

	USteamCoreProVoice* Obj = NewObject<USteamCoreProVoice>();
	Obj->AddToRoot();
	Obj->SetSampleRate(AudioSampleRate);

	return Obj;
}

USteamCoreProAsyncActionListenForControllerChange* USteamCoreProAsyncActionListenForControllerChange::ListenForControllerChange(UObject* WorldContextObject)
{
	LogSteamCoreVerbose("");

	const auto AsyncObject = NewObject<USteamCoreProAsyncActionListenForControllerChange>();
	AsyncObject->RegisterWithGameInstance(WorldContextObject->GetWorld()->GetGameInstance());

	AsyncObject->Activate();

	return AsyncObject;
}

void USteamCoreProAsyncActionListenForControllerChange::HandleCallback(bool bIsConnected, int32 PlatformUserId, int32 UserId)
{
	OnControllerChanged.Broadcast(bIsConnected, UserId);
}

void USteamCoreProAsyncActionListenForControllerChange::Activate()
{
	Super::Activate();

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	FCoreDelegates::OnControllerConnectionChange.AddWeakLambda(this, [this](bool bIsConnected, FPlatformUserId PlatformUserId, int32 UserId)
	{
		HandleCallback(bIsConnected, PlatformUserId, UserId);
	});
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FSteamItemInstanceID USteamUtilities::MakeSteamItemInstanceID(int64 Value)
{
	return FSteamItemInstanceID(Value);
}

FSteamID USteamUtilities::MakeSteamID(FString Value)
{
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FSteamID(TempId);
}

FSteamGameID USteamUtilities::MakeSteamGameID(FString Value)
{
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FSteamGameID(TempId);
}

int64 USteamUtilities::BreakSteamItemInstanceID(FSteamItemInstanceID SteamItemInstanceID)
{
	return SteamItemInstanceID;
}

FString USteamUtilities::BreakSteamID(FSteamID SteamID)
{
	return LexToString(static_cast<uint64>(SteamID));
}

FString USteamUtilities::BreakSteamGameID(FSteamGameID SteamID)
{
	return LexToString(static_cast<uint64>(SteamID));
}

FPublishedFileID USteamUtilities::MakePublishedFileID(FString Value)
{
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FPublishedFileID(TempId);
}

FSteamUGCHandle USteamUtilities::MakeUGCHandle(FString Value)
{
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FSteamUGCHandle(TempId);
}

FSteamInventoryUpdateHandle USteamUtilities::MakeInventoryUpdateHandle(FString Value)
{
#if WITH_STEAMCORE
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FSteamInventoryUpdateHandle(TempId);
#else
	return FSteamInventoryUpdateHandle();
#endif
}

FSteamTicketHandle USteamUtilities::MakeTicketHandle(FString Value)
{
	uint64 TempId = 0;
	LexFromString(TempId, *Value);
	return FSteamTicketHandle(TempId);
}

FString USteamUtilities::BreakPublishedFileID(FPublishedFileID FileID)
{
	return LexToString(static_cast<uint64>(FileID));
}

FString USteamUtilities::BreakUGCHandle(FSteamUGCHandle Handle)
{
	return LexToString(static_cast<uint64>(Handle));
}

FString USteamUtilities::BreakInventoryUpdateHandle(FSteamInventoryUpdateHandle Handle)
{
	return LexToString(static_cast<uint64>(Handle));
}

FString USteamUtilities::BreakTicketHandle(FSteamTicketHandle Handle)
{
	return LexToString(static_cast<uint64>(Handle));
}

bool USteamUtilities::IsValid(FSteamID SteamID)
{
#if WITH_STEAMCORE
	return CSteamID(SteamID.GetValue()).IsValid();
#else
	return false;
#endif
}

bool USteamUtilities::IsPublishedFileIDValid(FPublishedFileID PublishedFileId)
{
	return PublishedFileId.IsValid();
}

bool USteamUtilities::Equal(FSteamID A, FSteamID B)
{
	return A == B;
}

bool USteamUtilities::NotEqual(FSteamID A, FSteamID B)
{
	return A != B;
}

bool USteamUtilities::PublishedFileID_Equals(FPublishedFileID A, FPublishedFileID B)
{
	return A == B;
}

bool USteamUtilities::PublishedFileID_NotEquals(FPublishedFileID A, FPublishedFileID B)
{
	return A != B;
}

bool USteamUtilities::SteamItemInstanceID_Equals(FSteamItemInstanceID A, FSteamItemInstanceID B)
{
	return static_cast<uint64>(A) == static_cast<uint64>(B);
}

void USteamUtilities::SteamItemInstanceID_Equals_Exec(FSteamItemInstanceID A, FSteamItemInstanceID B, ESteamCoreProIdentical& Result)
{
	Result = static_cast<uint64>(A) == static_cast<uint64>(B) ? ESteamCoreProIdentical::Identical : ESteamCoreProIdentical::NotIdentical;
}

bool USteamUtilities::GetGameEngineInitialized()
{
	return (GEngine && GEngine->IsInitialized());
}

ESteamAccountType USteamUtilities::GetAccountType(FSteamID SteamID)
{
#if WITH_STEAMCORE
	return static_cast<ESteamAccountType>((CSteamID(SteamID.GetValue())).GetEAccountType());
#else
	return ESteamAccountType::Invalid;
#endif
}

bool USteamUtilities::IsLobby(FSteamID SteamID)
{
#if WITH_STEAMCORE
	return CSteamID(SteamID.GetValue()).IsLobby();
#else
	return false;
#endif
}

bool USteamUtilities::IsSteamAvailable()
{
#if WITH_STEAMCORE
	FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (SteamSubsystem)
	{
		return SteamSubsystem->IsSteamClientAvailable() || SteamSubsystem->IsSteamServerAvailable();
	}
#endif
	return false;
}

UServerFilter* USteamUtilities::ConstructServerFilter(UObject* WorldContextObject)
{
	UServerFilter* Object = nullptr;

	if (WorldContextObject)
	{
		Object = NewObject<UServerFilter>(WorldContextObject);

		if (Object)
		{
			Object->AddToRoot();
		}
	}
	else
	{
		UE_LOG(LogSteamCorePro, Error, TEXT("No world context object when creating UServerFilter object!"));
	}

	return Object;
}

FString USteamUtilities::EncryptString(FString String)
{
	FString Result;

	if (String.Len() > 0)
	{
		Result = FMD5::HashAnsiString(*String);
	}

	return Result;
}

void USteamUtilities::GetPublicIp(const FOnHTTPResponse& Callback)
{
	LogSteamCoreVeryVerbose("");

	FHttpModule* HttpModule = &FHttpModule::Get();

	if (HttpModule && HttpModule->IsHttpEnabled())
	{
		auto Request = HttpModule->CreateRequest();
		Request->SetVerb("GET");
		Request->SetURL(FString("http://api.ipify.org"));
		Request->SetHeader("User-Agent", "X-SteamCorePro/1.0");
		Request->SetHeader("Content-Type", "text/html");

		Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (Response)
			{
				Callback.ExecuteIfBound(Response->GetContentAsString());
			}
		});

		if (!Request->ProcessRequest())
		{
			LogSteamCoreWarn("Unable to send HTTP request");
		}
	}
}

FSteamID USteamUtilities::GetSteamIdFromPlayerState(APlayerState* PlayerState)
{
	FSteamID SteamId;

	if (PlayerState)
	{
#if !UE_VERSION_OLDER_THAN(4, 25, 0)
		SteamId = FSteamID(FCString::Atoi64(*PlayerState->GetUniqueId().ToString()));
#else
		SteamId = FSteamID(FCString::Atoi64(*PlayerState->UniqueId.ToString()));
#endif
	}

	return SteamId;
}

TArray<uint8> USteamUtilities::BP_StringToBytes(FString String)
{
	TArray<uint8> Array;
	Array.SetNum(String.Len());

	StringToBytes(String, Array.GetData(), String.Len());

	return Array;
}

FString USteamUtilities::BP_BytesToString(TArray<uint8> Array)
{
	return BytesToString(Array.GetData(), Array.Num());
}

bool USteamUtilities::IsSteamServerInitialized()
{
#if WITH_STEAMCORE
	if (SteamGameServerUtils() && SteamGameServer())
	{
		return SteamGameServer()->BLoggedOn();
	}
#endif

	return false;
}

TArray<uint8> USteamUtilities::K2_HexToBytes(FString String)
{
	TArray<uint8> Array;
	Array.SetNum(String.Len());

	HexToBytes(String, Array.GetData());

	return Array;
}

FString USteamUtilities::K2_HexToString(TArray<uint8> Array)
{
	return BytesToHex(Array.GetData(), Array.Num());
}

TArray<uint8> USteamUtilities::ReadFileToBytes(const FString& AbsoluteFilePath)
{
	LogSteamCoreVerbose("ReadFileToBytes (%s)", *AbsoluteFilePath);

	TArray<uint8> Result;

	if (FPaths::FileExists(*AbsoluteFilePath))
	{
		FFileHelper::LoadFileToArray(Result, *AbsoluteFilePath, 0);
	}
	else
	{
		LogSteamCoreError("File (%s) does not exist", *AbsoluteFilePath);
	}

	return Result;
}

bool USteamUtilities::WriteBytesToFile(bool bOverwriteIfExists, const FString& AbsoluteFilePath, const TArray<uint8>& DataBuffer)
{
	LogSteamCoreVerbose("WriteBytesToFile (%s)", *AbsoluteFilePath);

	if ((FPaths::FileExists(*AbsoluteFilePath) && bOverwriteIfExists) || !FPaths::FileExists(*AbsoluteFilePath))
	{
		return FFileHelper::SaveArrayToFile(DataBuffer, *AbsoluteFilePath);
	}
	else
	{
		LogSteamCoreError("File (%s) already exists", *AbsoluteFilePath);
	}

	return false;
}

bool USteamUtilities::TryJoinServer(FString& OutErrorMessage, const UObject* WorldContextObject, const FSteamServerAddr& ServerAddr, APlayerController* SpecificPlayer, bool bUsingSockets)
{
	LogSteamCoreVerbose("");

	FString ConsoleCommand;
	OutErrorMessage.Empty();

	if (bUsingSockets && (ServerAddr.SteamP2PAddr.IsEmpty() == false))
	{
		ConsoleCommand = FString::Printf(TEXT("open steam.%s:%d"), *ServerAddr.SteamP2PAddr, ServerAddr.Port);
	}
	else if (!bUsingSockets && (ServerAddr.IP.IsEmpty() == false))
	{
		ConsoleCommand = FString::Printf(TEXT("open %s:%d"), *ServerAddr.IP, ServerAddr.Port);
	}

	if (ConsoleCommand.IsEmpty())
	{
		OutErrorMessage = FString("No valid server specified, either missing IP or P2P address");
		return false;
	}

	UKismetSystemLibrary::ExecuteConsoleCommand(WorldContextObject, ConsoleCommand, SpecificPlayer);

	return true;
}

bool USteamUtilities::KickPlayer(UObject* WorldContextObject, APlayerController* KickedPlayer, const FText& KickReason)
{
	if (WorldContextObject)
	{
		if (const AGameModeBase* GameMode = WorldContextObject->GetWorld()->GetAuthGameMode())
		{
			if (GameMode->GameSession)
			{
				return GameMode->GameSession->KickPlayer(KickedPlayer, KickReason);
			}
		}
	}
	return false;
}

bool USteamUtilities::K2_IsPlayerInSession(int32 LocalUserNum)
{
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetSessionInterface())
	{
		const FUniqueNetIdPtr UserId = SteamSubsystem->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
		return SteamSubsystem->GetSessionInterface()->IsPlayerInSession(NAME_GameSession, *UserId);
	}
#endif

	return false;
}

FString USteamUtilities::GetAccountId(const FSteamID& SteamId)
{
	LogSteamCoreVeryVerbose("");
	FString Result;
	
#if WITH_STEAMCORE
	const CSteamID& CSteamId = SteamId;
	Result = LexToString(CSteamId.GetAccountID());
#endif

	return Result;
}

bool USteamUtilities::IsUsingP2PRelays()
{
	bool bResult = false;
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (SteamSubsystem && SteamSubsystem->GetPingInterface())
	{
		bResult = SteamSubsystem->GetPingInterface()->IsUsingP2PRelays();
	}

#endif

	return bResult;
}

FString USteamUtilities::GetHostPingData()
{
	FString Result;
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (SteamSubsystem && SteamSubsystem->GetPingInterface())
	{
		Result = SteamSubsystem->GetPingInterface()->GetHostPingData();
	}
#endif

	return Result;
}

int32 USteamUtilities::GetPingFromHostData(const FString& Data)
{
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (SteamSubsystem && SteamSubsystem->GetPingInterface())
	{
		return SteamSubsystem->GetPingInterface()->GetPingFromHostData(Data);
	}
#endif

	return 0;
}

bool USteamUtilities::IsRecalculatingPing()
{
	bool bResult = false;
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));
	if (SteamSubsystem && SteamSubsystem->GetPingInterface())
	{
		bResult = SteamSubsystem->GetPingInterface()->IsRecalculatingPing();
	}

#endif

	return bResult;
}

bool USteamUtilities::MuteRemoteTalker(UObject* WorldContextObject, int32 LocalUserNum, APlayerState* PlayerState, bool bIsSystemWide)
{
	if (PlayerState && WorldContextObject)
	{
		return UOnlineEngineInterface::Get()->MuteRemoteTalker(WorldContextObject->GetWorld(), LocalUserNum, *PlayerState->GetUniqueId(), false);
	}

	return false;
}

bool USteamUtilities::UnmuteRemoteTalker(UObject* WorldContextObject, int32 LocalUserNum, APlayerState* PlayerState, bool bIsSystemWide)
{
	if (PlayerState && WorldContextObject)
	{
		return UOnlineEngineInterface::Get()->UnmuteRemoteTalker(WorldContextObject->GetWorld(), LocalUserNum, *PlayerState->GetUniqueId(), false);
	}

	return false;
}

bool USteamUtilities::IsPlayerMuted(UObject* WorldContextObject, APlayerState* Player)
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		const IOnlineVoicePtr VoiceInt = Subsystem->GetVoiceInterface();
		
		if (VoiceInt)
		{
			return VoiceInt->IsMuted(0, *Player->GetUniqueId());
		}
	}
	return false;
}

bool USteamUtilities::IsRemotePlayerTalking(UObject* WorldContextObject, APlayerState* Player)
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		const IOnlineVoicePtr VoiceInt = Subsystem->GetVoiceInterface();
		
		if (VoiceInt)
		{
			return VoiceInt->IsRemotePlayerTalking(*Player->GetUniqueId());
		}
	}
	return false;
}

bool USteamUtilities::IsLocalPlayerTalking(UObject* WorldContextObject, int32 LocalUserNum)
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		const IOnlineVoicePtr VoiceInt = Subsystem->GetVoiceInterface();
		
		if (VoiceInt)
		{
			return VoiceInt->IsLocalPlayerTalking(LocalUserNum);
		}
	}
	return false;
}

void USteamUtilities::UpdateVoiceInputDevice(FString DeviceId)
{
	GConfig->SetString(TEXT("OnlineSubsystemSteamCore"), TEXT("VoiceInput"), *DeviceId, GGameIni);
	GConfig->Flush(false, GGameIni);
}

FString USteamUtilities::GetVoiceInputDevice()
{
	FString Result;
	GConfig->GetString(TEXT("OnlineSubsystemSteamCore"), TEXT("VoiceInput"), Result, GGameIni);

	return Result;
}

void USteamUtilities::ListenForSteamMessages(const FOnSteamMessage& Callback)
{
	s_SteamMessageListeners.Add(Callback);
}

void USteamUtilities::InvokeSteamMessage(ESteamMessageType Type, const FString& Message)
{
	for (auto& Element : s_SteamMessageListeners)
	{
		Element.ExecuteIfBound(Type, Message);
	}
}

void USteamUtilities::ListenForSessionInviteAccepted(const FOnSessionUserInviteAcceptedCallback& Callback)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetSessionInterface())
	{
		s_SessionInviteAcceptedDelegateHandle = SteamSubsystem->GetSessionInterface()->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateLambda([Callback](const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
		{
			FBlueprintSessionResult SessionResult;
			SessionResult.OnlineResult = InviteResult;

			Callback.ExecuteIfBound(bWasSuccessful, ControllerId, UserId->ToString(), SessionResult);
		}));
	}
#endif
}

void USteamUtilities::StopListeningForSessionInviteAccepted()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetSessionInterface())
	{
		SteamSubsystem->GetSessionInterface()->ClearOnSessionUserInviteAcceptedDelegate_Handle(s_SessionInviteAcceptedDelegateHandle);
		s_SessionInviteAcceptedDelegateHandle.Reset();
	}
#endif
}

/*
void USteamUtilities::ListenForSessionInviteRecieved(const FOnSessionUserInviteRecievedCallback& Callback)
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetSessionInterface())
	{
		s_SessionInviteReceivedDelegateHandle = SteamSubsystem->GetSessionInterface()->AddOnSessionInviteReceivedDelegate_Handle(FOnSessionInviteReceivedDelegate::CreateLambda([Callback](const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
		{
			FBlueprintSessionResult SessionResult;
			SessionResult.OnlineResult = InviteResult;
			
			Callback.ExecuteIfBound(SessionResult);
		}));
	}
#endif
}

void USteamUtilities::StopListeningForSessionInviteRecieved()
{
	LogSteamCoreVerbose("");
	
#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetSessionInterface())
	{
		SteamSubsystem->GetSessionInterface()->ClearOnSessionInviteReceivedDelegate_Handle(s_SessionInviteReceivedDelegateHandle);
		s_SessionInviteReceivedDelegateHandle.Reset();
	}
#endif
}
*/

TArray<FOnlineFriendSteamCoreBlueprint> USteamUtilities::GetFriendsList(EFriendListType FriendListType)
{
	LogSteamCoreVerbose("");

	TArray<FOnlineFriendSteamCoreBlueprint> Result;

#if WITH_STEAMCORE
	const FOnlineSubsystemSteamCore* SteamSubsystem = static_cast<FOnlineSubsystemSteamCore*>(IOnlineSubsystem::Get(STEAMCORE_SUBSYSTEM));

	if (SteamSubsystem && SteamSubsystem->GetFriendsInterface())
	{
		TArray<TSharedRef<FOnlineFriend>> OutFriends;

		SteamSubsystem->GetFriendsInterface()->GetFriendsList(0, EFriendsLists::ToString((static_cast<EFriendsLists::Type>(FriendListType))), OutFriends);

		for (int32 i = 0; i < OutFriends.Num(); i++)
		{
			Result.Add(*OutFriends[i]);
		}
	}
#endif

	return Result;
}

void USteamUtilities::ReadFriendList(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EFriendListType FriendListType)
{
	LogSteamCoreVerbose("");

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FReadFriendListLatent>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FReadFriendListLatent(LatentInfo, FriendListType));
		}
	}
}
