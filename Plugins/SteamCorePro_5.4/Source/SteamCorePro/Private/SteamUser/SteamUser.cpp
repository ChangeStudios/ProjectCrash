/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUser
*/

#include "SteamUser/SteamUser.h"
#include "SteamUser/SteamUserAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProUser::USteamProUser()
{
#if WITH_STEAMCORE
	OnClientGameServerDenyCallbackCallback.Register(this, &USteamProUser::OnClientGameServerDeny);
	OnGameWebCallbackCallback.Register(this, &USteamProUser::OnGameWebCallback);
	OnGetAuthSessionTicketResponseCallback.Register(this, &USteamProUser::OnGetAuthSessionTicketResponse);
	OnIPCFailureCallback.Register(this, &USteamProUser::OnIPCFailure);
	OnLicensesUpdatedCallback.Register(this, &USteamProUser::OnLicensesUpdated);
	OnMicroTxnAuthorizationResponseCallback.Register(this, &USteamProUser::OnMicroTxnAuthorizationResponse);
	OnSteamServersConnectedCallback.Register(this, &USteamProUser::OnSteamServersConnected);
	OnSteamServerConnectFailureCallback.Register(this, &USteamProUser::OnSteamServerConnectFailure);
	OnSteamServersDisconnectedCallback.Register(this, &USteamProUser::OnSteamServersDisconnected);
	OnValidateAuthTicketResponseCallback.Register(this, &USteamProUser::OnValidateAuthTicketResponse);
	OnEncryptedAppTicketResponseCallback.Register(this, &USteamProUser::OnEncryptedAppTicketResponse);
	OnGetTicketForWebApiResponseCallback.Register(this, &USteamProUser::OnGetTicketForWebApiResponse);

	if (IsRunningDedicatedServer())
	{
		OnClientGameServerDenyCallbackCallback.SetGameserverFlag();
		OnGameWebCallbackCallback.SetGameserverFlag();
		OnGetAuthSessionTicketResponseCallback.SetGameserverFlag();
		OnIPCFailureCallback.SetGameserverFlag();
		OnLicensesUpdatedCallback.SetGameserverFlag();
		OnMicroTxnAuthorizationResponseCallback.SetGameserverFlag();
		OnSteamServersConnectedCallback.SetGameserverFlag();
		OnSteamServerConnectFailureCallback.SetGameserverFlag();
		OnSteamServersDisconnectedCallback.SetGameserverFlag();
		OnValidateAuthTicketResponseCallback.SetGameserverFlag();
		OnEncryptedAppTicketResponseCallback.SetGameserverFlag();
		OnGetTicketForWebApiResponseCallback.SetGameserverFlag();
	}
#endif
}

USteamProUser::~USteamProUser()
{
#if WITH_STEAMCORE
	OnClientGameServerDenyCallbackCallback.Unregister();
	OnGameWebCallbackCallback.Unregister();
	OnGetAuthSessionTicketResponseCallback.Unregister();
	OnIPCFailureCallback.Unregister();
	OnLicensesUpdatedCallback.Unregister();
	OnMicroTxnAuthorizationResponseCallback.Unregister();
	OnSteamServersConnectedCallback.Unregister();
	OnSteamServerConnectFailureCallback.Unregister();
	OnSteamServersDisconnectedCallback.Unregister();
	OnValidateAuthTicketResponseCallback.Unregister();
	OnEncryptedAppTicketResponseCallback.Unregister();
	OnGetTicketForWebApiResponseCallback.Unregister();
#endif
}

USteamProUser* USteamProUser::GetSteamUser()
{
#if WITH_STEAMCORE
	return SteamUser() ? ThisClass::StaticClass()->GetDefaultObject<USteamProUser>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void USteamProUser::AdvertiseGame(FSteamID SteamIDGameServer, FString ServerIP, int32 ServerPort)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		FIPv4Address IP;
		FIPv4Address::Parse(ServerIP, IP);

		SteamUser()->AdvertiseGame(SteamIDGameServer, IP.Value, ServerPort);
	}
#endif
}

ESteamBeginAuthSessionResult USteamProUser::BeginAuthSession(TArray<uint8> Ticket, FSteamID SteamID)
{
	LogSteamCoreVerbose("");

	ESteamBeginAuthSessionResult Result = ESteamBeginAuthSessionResult::InvalidTicket;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = static_cast<ESteamBeginAuthSessionResult>(SteamUser()->BeginAuthSession(Ticket.GetData(), Ticket.Num(), SteamID));
	}
#endif

	return Result;
}

bool USteamProUser::BIsBehindNAT()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BIsBehindNAT();
	}
#endif

	return bResult;
}

bool USteamProUser::BIsPhoneIdentifying()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BIsPhoneIdentifying();
	}
#endif

	return bResult;
}

bool USteamProUser::BIsPhoneRequiringVerification()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BIsPhoneRequiringVerification();
	}
#endif

	return bResult;
}

bool USteamProUser::BIsPhoneVerified()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BIsPhoneVerified();
	}
#endif

	return bResult;
}

bool USteamProUser::BIsTwoFactorEnabled()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BIsTwoFactorEnabled();
	}
#endif

	return bResult;
}

bool USteamProUser::BLoggedOn()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		bResult = SteamUser()->BLoggedOn();
	}
#endif

	return bResult;
}

void USteamProUser::CancelAuthTicket(FSteamTicketHandle TicketHandle)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		SteamUser()->CancelAuthTicket(TicketHandle);
	}
#endif
}

ESteamVoiceResult USteamProUser::DecompressVoice(const TArray<uint8>& CompressedBuffer, int32 DesiredSampleRate, TArray<uint8>& DestBuffer)
{
	LogSteamCoreVeryVerbose("");

	ESteamVoiceResult Result = ESteamVoiceResult::NotInitialized;
	DestBuffer.Empty();

#if WITH_STEAMCORE
	if (SteamUser())
	{
		uint32 BytesWritten = 0;
		DestBuffer.SetNum(1024 * 20);

		Result = static_cast<ESteamVoiceResult>(SteamUser()->DecompressVoice(CompressedBuffer.GetData(), CompressedBuffer.Num(), DestBuffer.GetData(), DestBuffer.Num(), &BytesWritten, DesiredSampleRate));

		if (Result == ESteamVoiceResult::BufferTooSmall)
		{
			DestBuffer.Empty();
			DestBuffer.SetNum(BytesWritten);

			Result = static_cast<ESteamVoiceResult>(SteamUser()->DecompressVoice(CompressedBuffer.GetData(), CompressedBuffer.Num(), DestBuffer.GetData(), DestBuffer.Num(), &BytesWritten, DesiredSampleRate));
		}

		DestBuffer.SetNum(BytesWritten);

		//LogSteamCoreVerbose("Bytes Written: %d, destBuffer: %d", BytesWritten, destBuffer.Num());
	}
#endif

	return Result;
}

void USteamProUser::EndAuthSession(FSteamID SteamID)
{
	LogSteamCoreVerbose("");
	
	#if WITH_STEAMCORE
	if (SteamUser())
	{
		SteamUser()->EndAuthSession(SteamID);
	}
#endif
}

FSteamTicketHandle USteamProUser::GetAuthSessionTicket(TArray<uint8>& OutTicket, FSteamNetworkingIdentity SteamNetworkingIdentity)
{
	LogSteamCoreVeryVerbose(""); 

	FSteamTicketHandle Result;
	OutTicket.Empty();

#if WITH_STEAMCORE
	if (SteamUser())
	{
		uint32 TicketSize = 0;
		OutTicket.SetNum(8192);
		Result = SteamUser()->GetAuthSessionTicket(OutTicket.GetData(), 8192, &TicketSize, &SteamNetworkingIdentity.m_SteamNetworkingIdentity);

		OutTicket.SetNum(TicketSize);
	}
#endif

	return Result;
}

FSteamTicketHandle USteamProUser::GetAuthTicketForWebApi(FString Identity)
{
	LogSteamCoreVeryVerbose(""); 

	FSteamTicketHandle Result;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = SteamUser()->GetAuthTicketForWebApi(TCHAR_TO_UTF8(*Identity));
	}
#endif

	return Result;
}

ESteamVoiceResult USteamProUser::GetAvailableVoice(int32& OutCompressedBytes, int32& OutUncompressedBytes, int32 UncompressedVoiceDesiredSampleRate)
{
	LogSteamCoreVeryVerbose("");

	ESteamVoiceResult Result = ESteamVoiceResult::NotInitialized;
	uint32 CompressedBytes = 0;
	uint32 UncompressedBytes = 0;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = static_cast<ESteamVoiceResult>(SteamUser()->GetAvailableVoice(&CompressedBytes, &UncompressedBytes, UncompressedVoiceDesiredSampleRate));
	}

	OutCompressedBytes = CompressedBytes;
	OutUncompressedBytes = UncompressedBytes;
#endif

	return Result;
}

bool USteamProUser::GetEncryptedAppTicket(TArray<uint8>& OutTicket)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutTicket.Empty();

#if WITH_STEAMCORE
	if (SteamUser())
	{
		uint32 TicketSize = 0;
		OutTicket.SetNum(8192);
		bResult = SteamUser()->GetEncryptedAppTicket(OutTicket.GetData(), OutTicket.Num(), &TicketSize);
		OutTicket.SetNum(TicketSize);
	}
#endif

	return bResult;
}

int32 USteamProUser::GetGameBadgeLevel(int32 Series, bool bFoil)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = SteamUser()->GetGameBadgeLevel(Series, bFoil);
	}
#endif

	return Result;
}

int32 USteamProUser::GetPlayerSteamLevel()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = SteamUser()->GetPlayerSteamLevel();
	}
#endif

	return Result;
}

FSteamID USteamProUser::GetSteamID()
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = FSteamID(SteamUser()->GetSteamID());
	}
#endif

	return Result;
}

FString USteamProUser::GetSteamIDAsString()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = FString::Printf(TEXT("%llu"), SteamUser()->GetSteamID().ConvertToUint64());
	}
#endif

	return Result;
}

ESteamVoiceResult USteamProUser::GetVoice(TArray<uint8>& OutDestBuffer, int32& OutBytesWritten)
{
	LogSteamCoreVeryVerbose("");

	ESteamVoiceResult Result = ESteamVoiceResult::NotInitialized;
	OutDestBuffer.Empty();
	uint32 BytesWritten = 0;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		// get the required buffer size
		uint32 AvailableVoiceBufferSize = 0;
		SteamUser()->GetAvailableVoice(&AvailableVoiceBufferSize);

		OutDestBuffer.SetNum(AvailableVoiceBufferSize);

		Result = static_cast<ESteamVoiceResult>(SteamUser()->GetVoice(true, OutDestBuffer.GetData(), AvailableVoiceBufferSize, &BytesWritten));
		OutDestBuffer.SetNum(BytesWritten);
	}
#endif

	OutBytesWritten = BytesWritten;

	return Result;
}

int32 USteamProUser::GetVoiceOptimalSampleRate()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = SteamUser()->GetVoiceOptimalSampleRate();
	}
#endif

	return Result;
}

void USteamProUser::RequestEncryptedAppTicket(const FOnRequestEncryptedAppTicket& Callback, TArray<uint8> DataToInclude)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket* Task = new FOnlineAsyncTaskSteamCoreProUserRequestEncryptedAppTicket(Callback, DataToInclude);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProUser::RequestStoreAuthURL(const FOnStoreAuthURLResponse& Callback, FString RedirectURL)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL* Task = new FOnlineAsyncTaskSteamCoreProUserRequestStoreAuthURL(Callback, RedirectURL);
		QueueAsyncTask(Task);
	}
#endif
}

void USteamProUser::StartVoiceRecording()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		SteamUser()->StartVoiceRecording();
	}
#endif
}

void USteamProUser::StopVoiceRecording()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamUser())
	{
		SteamUser()->StopVoiceRecording();
	}
#endif
}

ESteamUserHasLicenseForAppResult USteamProUser::UserHasLicenseForApp(FSteamID SteamID, int32 AppID)
{
	LogSteamCoreVerbose("");

	ESteamUserHasLicenseForAppResult Result = ESteamUserHasLicenseForAppResult::NoAuth;

#if WITH_STEAMCORE
	if (SteamUser())
	{
		Result = static_cast<ESteamUserHasLicenseForAppResult>(SteamUser()->UserHasLicenseForApp(SteamID, AppID));
	}
#endif

	return Result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProUser::OnClientGameServerDeny(ClientGameServerDeny_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ClientGameServerDeny.Broadcast(Data);
	});
}

void USteamProUser::OnGameWebCallback(GameWebCallback_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GameWebCallback.Broadcast(Data);
	});
}

void USteamProUser::OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GetAuthSessionTicketResponse.Broadcast(Data);
	});
}

void USteamProUser::OnIPCFailure(IPCFailure_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		IpcFailure.Broadcast(Data);
	});
}

void USteamProUser::OnLicensesUpdated(LicensesUpdated_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LicensesUpdated.Broadcast(Data);
	});
}

void USteamProUser::OnMicroTxnAuthorizationResponse(MicroTxnAuthorizationResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		MicroTxnAuthorizationResponse.Broadcast(Data);
	});
}

void USteamProUser::OnSteamServersConnected(SteamServersConnected_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamServersConnected.Broadcast(Data);
	});
}

void USteamProUser::OnSteamServerConnectFailure(SteamServerConnectFailure_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamServerConnectFailure.Broadcast(Data);
	});
}

void USteamProUser::OnSteamServersDisconnected(SteamServersDisconnected_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamServersDisconnected.Broadcast(Data);
	});
}

void USteamProUser::OnValidateAuthTicketResponse(ValidateAuthTicketResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ValidateAuthTicketResponse.Broadcast(Data);
	});
}

void USteamProUser::OnEncryptedAppTicketResponse(EncryptedAppTicketResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		EncryptedAppTicketResponse.Broadcast(Data);
	});
}

void USteamProUser::OnGetTicketForWebApiResponse(GetTicketForWebApiResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GetTicketForWebApiResponse.Broadcast(Data);
	});
}
#endif
