/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamNetworking
*/

#include "SteamNetworking/SteamNetworking.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProNetworking::USteamProNetworking()
{
#if WITH_STEAMCORE
	OnP2PSessionRequestCallback.Register(this, &USteamProNetworking::OnP2PSessionRequest);
	OnP2PSessionConnectFailCallback.Register(this, &USteamProNetworking::OnP2PSessionConnectFail);

	if (IsRunningDedicatedServer())
	{
		OnP2PSessionRequestCallback.SetGameserverFlag();
		OnP2PSessionConnectFailCallback.SetGameserverFlag();
	}
#endif
}

USteamProNetworking::~USteamProNetworking()
{
#if WITH_STEAMCORE
	OnP2PSessionRequestCallback.Unregister();
	OnP2PSessionConnectFailCallback.Unregister();
#endif
}

USteamProNetworking* USteamProNetworking::GetSteamNetworking()
{
#if WITH_STEAMCORE
	return SteamNetworking() ? ThisClass::StaticClass()->GetDefaultObject<USteamProNetworking>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
bool USteamProNetworking::AcceptP2PSessionWithUser(FSteamID SteamIDRemote)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->AcceptP2PSessionWithUser(SteamIDRemote);
	}
#endif

	return bResult;
}

bool USteamProNetworking::AllowP2PPacketRelay(bool bAllow)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->AllowP2PPacketRelay(bAllow);
	}
#endif

	return bResult;
}

bool USteamProNetworking::CloseP2PChannelWithUser(FSteamID SteamIDRemote, int32 Channel)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->CloseP2PChannelWithUser(SteamIDRemote, Channel);
	}
#endif

	return bResult;
}

bool USteamProNetworking::CloseP2PSessionWithUser(FSteamID SteamIDRemote)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->CloseP2PSessionWithUser(SteamIDRemote);
	}
#endif

	return bResult;
}

bool USteamProNetworking::GetP2PSessionState(FSteamID SteamIDRemote, FSteamP2PSessionState& ConnectionState)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	ConnectionState = FSteamP2PSessionState();

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		P2PSessionState_t SessionState;

		bResult = GetNetworking()->GetP2PSessionState(SteamIDRemote, &SessionState);

		if (bResult)
		{
			ConnectionState = SessionState;
		}
	}
#endif

	return bResult;
}

bool USteamProNetworking::IsP2PPacketAvailable(int32& OutMessageSize, int32 Channel)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	
	OutMessageSize = 0;
	uint32 MessageSize = 0;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->IsP2PPacketAvailable(&MessageSize, Channel);
		
		if (bResult)
		{
			OutMessageSize = MessageSize;
		}
	}
#endif

	return bResult;
}

bool USteamProNetworking::ReadP2PPacket(TArray<uint8>& Data, FSteamID& OutSteamIdRemote, int32 MessageSize, int32 Channel)
{
	LogSteamCoreVerbose("");

	bool bResult = false;
	Data.Empty();
	OutSteamIdRemote = FSteamID();

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		Data.SetNum(MessageSize);
		CSteamID SteamIdRemote;

		uint32 ReturnedMessageSize = 0;

		bResult = GetNetworking()->ReadP2PPacket(Data.GetData(), MessageSize, &ReturnedMessageSize, &SteamIdRemote, Channel);

		if (bResult)
		{
			Data.SetNum(ReturnedMessageSize);
			OutSteamIdRemote = SteamIdRemote;
		}
		else
		{
			Data.Empty();
		}
	}
#endif

	return bResult;
}

bool USteamProNetworking::SendP2PPacket(FSteamID SteamIDRemote, TArray<uint8> Data, ESteamP2PSend P2PSendType, int32 Channel)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetNetworking())
	{
		bResult = GetNetworking()->SendP2PPacket(SteamIDRemote, Data.GetData(), Data.Num(), static_cast<EP2PSend>(P2PSendType), Channel);
	}
#endif

	return bResult;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProNetworking::OnP2PSessionRequest(P2PSessionRequest_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		OnP2PSessionRequestDelegate.Broadcast(Data);
	});
}

void USteamProNetworking::OnP2PSessionConnectFail(P2PSessionConnectFail_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		OnP2PSessionConnectFailDelegate.Broadcast(Data);
	});
}
#endif
