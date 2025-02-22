/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamGameServer
*/

#include "SteamGameServer/SteamProGameServer.h"
#include "SteamGameServer/SteamGameServerAsyncTasks.h"
#include "SteamCoreProPluginPrivatePCH.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

USteamProGameServer::USteamProGameServer()
{
#if WITH_STEAMCORE
	OnGSPolicyResponseCallback.Register(this, &USteamProGameServer::OnGSPolicyResponse);
	OnGSClientGroupStatusCallback.Register(this, &USteamProGameServer::OnGSClientGroupStatus);
	OnGSValidateAuthTicketResponseCallback.Register(this, &USteamProGameServer::OnGSValidateAuthTicketResponse);
	OnGSClientApproveCallback.Register(this, &USteamProGameServer::OnGSClientApprove);
	OnGSClientDenyCallback.Register(this, &USteamProGameServer::OnGSClientDeny);

	if (IsRunningDedicatedServer())
	{
		OnGSPolicyResponseCallback.SetGameserverFlag();
		OnGSClientGroupStatusCallback.SetGameserverFlag();
		OnGSValidateAuthTicketResponseCallback.SetGameserverFlag();
		OnGSClientApproveCallback.SetGameserverFlag();
		OnGSClientDenyCallback.SetGameserverFlag();
	}
#endif
}

USteamProGameServer::~USteamProGameServer()
{
#if WITH_STEAMCORE
	OnGSPolicyResponseCallback.Unregister();
	OnGSClientGroupStatusCallback.Unregister();
	OnGSValidateAuthTicketResponseCallback.Unregister();
	OnGSClientApproveCallback.Unregister();
	OnGSClientDenyCallback.Unregister();
#endif
}

USteamProGameServer* USteamProGameServer::GetSteamGameServer()
{
#if WITH_STEAMCORE
	return SteamGameServer() ? ThisClass::StaticClass()->GetDefaultObject<USteamProGameServer>() : nullptr;
#endif
	return nullptr;
}

void USteamProGameServer::AssociateWithClan(const FOnAssociateWithClan& Callback, FSteamID SteamIDClan)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan* Task = new FOnlineAsyncTaskSteamCoreProGameServerAssociateWithClan(Callback, SteamIDClan);
		QueueAsyncTask(Task);
	}
#endif
}

ESteamBeginAuthSessionResult USteamProGameServer::BeginAuthSession(TArray<uint8> Ticket, FSteamID SteamID)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	ESteamBeginAuthSessionResult Result = ESteamBeginAuthSessionResult::InvalidTicket;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		Result = static_cast<ESteamBeginAuthSessionResult>(SteamGameServer()->BeginAuthSession(Ticket.GetData(), Ticket.Num(), SteamID));
	}
#endif

	return Result;
}

bool USteamProGameServer::BLoggedOn()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		bResult = SteamGameServer()->BLoggedOn();
	}
#endif

	return bResult;
}

bool USteamProGameServer::BSecure()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		bResult = SteamGameServer()->BSecure();
	}
#endif

	return bResult;
}

bool USteamProGameServer::BUpdateUserData(FSteamID SteamIDUser, FString PlayerName, int32 Score)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		bResult = SteamGameServer()->BUpdateUserData(SteamIDUser, TCHAR_TO_UTF8(*PlayerName), Score);
	}
#endif

	return bResult;
}

void USteamProGameServer::CancelAuthTicket(FSteamTicketHandle TicketHandle)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->CancelAuthTicket(TicketHandle);
	}
#endif
}

void USteamProGameServer::ClearAllKeyValues()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->ClearAllKeyValues();
	}
#endif
}

void USteamProGameServer::ComputeNewPlayerCompatibility(const FOnComputeNewPlayerCompatibility& Callback, FSteamID SteamIDNewPlayer)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility* Task = new FOnlineAsyncTaskSteamCoreProGameServerComputeNewPlayerCompatibility(Callback, SteamIDNewPlayer);
		QueueAsyncTask(Task);
	}
#endif
}

FSteamID USteamProGameServer::CreateUnauthenticatedUserConnection()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		Result = SteamGameServer()->CreateUnauthenticatedUserConnection();
	}
#endif

	return Result;
}

void USteamProGameServer::SetAdvertiseServerActive(bool bActive)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetAdvertiseServerActive(bActive);
	}
#endif
}

void USteamProGameServer::EndAuthSession(FSteamID SteamID)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->EndAuthSession(SteamID);
	}
#endif
}

FSteamTicketHandle USteamProGameServer::GetAuthSessionTicket(TArray<uint8>& Ticket, FSteamNetworkingIdentity SteamNetworkingIdentity)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	FSteamTicketHandle TicketHandle;
	Ticket.Empty();

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		uint32 TicketSize = 0;
		TicketHandle = SteamGameServer()->GetAuthSessionTicket(Ticket.GetData(), 8192, &TicketSize, &SteamNetworkingIdentity.m_SteamNetworkingIdentity);
		Ticket.SetNum(TicketSize);
	}
#endif

	return TicketHandle;
}

FString USteamProGameServer::GetServerPublicIP()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	FString Result = FString("INVALID");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		Result = FIPv4Address(SteamGameServer()->GetPublicIP().m_unIPv4).ToString();
	}
#endif

	return Result;
}

FSteamID USteamProGameServer::GetServerSteamID()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		Result = FSteamID(SteamGameServer()->GetSteamID().ConvertToUint64());
	}
#endif

	return Result;
}

void USteamProGameServer::LogOff()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->LogOff();
	}
#endif
}

void USteamProGameServer::LogOn(FString Token)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->LogOn(TCHAR_TO_UTF8(*Token));
	}
#endif
}

void USteamProGameServer::LogOnAnonymous()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->LogOnAnonymous();
	}
#endif
}

bool USteamProGameServer::RequestUserGroupStatus(FSteamID SteamIDUser, FSteamID SteamIDGroup)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		bResult = SteamGameServer()->RequestUserGroupStatus(SteamIDUser, SteamIDGroup);
	}
#endif

	return bResult;
}

void USteamProGameServer::SetBotPlayerCount(int32 Botplayers)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetBotPlayerCount(Botplayers);
	}
#endif
}

void USteamProGameServer::SetDedicatedServer(bool bDedicated)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetDedicatedServer(bDedicated);
	}
#endif
}

void USteamProGameServer::SetGameData(FString GameData)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetGameData(TCHAR_TO_UTF8(*GameData));
	}
#endif
}

void USteamProGameServer::SetGameDescription(FString GameDescription)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetGameDescription(TCHAR_TO_UTF8(*GameDescription));
	}
#endif
}

void USteamProGameServer::SetGameTags(FString GameTags)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetGameTags(TCHAR_TO_UTF8(*GameTags));
	}
#endif
}

void USteamProGameServer::SetKeyValue(FString Key, FString Value)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	if (Key.Len() == 0 || Value.Len() == 0)
	{
		return;
	}

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		const FTCHARToUTF8 KeyChar(*Key);
		const FTCHARToUTF8 ValueChar(*Value);

		SteamGameServer()->SetKeyValue(KeyChar.Get(), ValueChar.Get());
	}
#endif
}

void USteamProGameServer::SetMapName(FString MapName)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetMapName(TCHAR_TO_UTF8(*MapName));
	}
#endif
}

void USteamProGameServer::SetMaxPlayerCount(int32 PlayersMax)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetMaxPlayerCount(PlayersMax);
	}
#endif
}

void USteamProGameServer::SetModDir(FString ModDir)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetModDir(TCHAR_TO_UTF8(*ModDir));
	}
#endif
}

void USteamProGameServer::SetPasswordProtected(bool bPasswordProtected)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetPasswordProtected(bPasswordProtected);
	}
#endif
}

void USteamProGameServer::SetProduct(FString Product)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetProduct(TCHAR_TO_UTF8(*Product));
	}
#endif
}

void USteamProGameServer::SetRegion(FString Region)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetRegion(TCHAR_TO_UTF8(*Region));
	}
#endif
}

void USteamProGameServer::SetServerName(FString ServerName)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetServerName(TCHAR_TO_UTF8(*ServerName));
	}
#endif
}

void USteamProGameServer::SetSpectatorPort(int32 SpectatorPort)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetSpectatorPort(SpectatorPort);
	}
#endif
}

void USteamProGameServer::SetSpectatorServerName(FString SpectatorServerName)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		SteamGameServer()->SetSpectatorServerName(TCHAR_TO_UTF8(*SpectatorServerName));
	}
#endif
}

ESteamUserHasLicenseForAppResult USteamProGameServer::UserHasLicenseForApp(FSteamID SteamID, int32 AppID)
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	ESteamUserHasLicenseForAppResult Result = ESteamUserHasLicenseForAppResult::DoesNotHaveLicense;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		Result = static_cast<ESteamUserHasLicenseForAppResult>(SteamGameServer()->UserHasLicenseForApp(SteamID, AppID));
	}
#endif

	return Result;
}

bool USteamProGameServer::WasRestartRequested()
{
	if (!IsRunningDedicatedServer())
	{
		LogSteamCoreError("This function can only be called on a Dedicated Server!");
	}

	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamGameServer())
	{
		bResult = SteamGameServer()->WasRestartRequested();
	}
#endif

	return bResult;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProGameServer::OnGSPolicyResponse(GSPolicyResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSPolicyResponse.Broadcast(Data);
	});
}

void USteamProGameServer::OnGSClientGroupStatus(GSClientGroupStatus_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSClientGroupStatus.Broadcast(Data);
	});
}

void USteamProGameServer::OnGSValidateAuthTicketResponse(ValidateAuthTicketResponse_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSValidateAuthTicketResponse.Broadcast(Data);
	});
}

void USteamProGameServer::OnGSClientApprove(GSClientApprove_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSClientApprove.Broadcast(Data);
	});
}

void USteamProGameServer::OnGSClientDeny(GSClientDeny_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GSClientDeny.Broadcast(Data);
	});
}
#endif
