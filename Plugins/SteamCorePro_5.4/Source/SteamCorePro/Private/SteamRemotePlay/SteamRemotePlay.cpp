/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamRemotePlay
*/

#include "SteamRemotePlay/SteamRemotePlay.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProRemotePlay::USteamProRemotePlay()
{
#if WITH_STEAMCORE
	OnSteamRemotePlaySessionConnectedCallback.Register(this, &USteamProRemotePlay::OnSteamRemotePlaySessionConnected);
	OnSteamRemotePlaySessionDisconnectedCallback.Register(this, &USteamProRemotePlay::OnSteamRemotePlaySessionDisconnected);

	if (IsRunningDedicatedServer())
	{
		OnSteamRemotePlaySessionConnectedCallback.SetGameserverFlag();
		OnSteamRemotePlaySessionDisconnectedCallback.SetGameserverFlag();
	}
#endif
}

USteamProRemotePlay::~USteamProRemotePlay()
{
#if WITH_STEAMCORE
	OnSteamRemotePlaySessionConnectedCallback.Unregister();
	OnSteamRemotePlaySessionDisconnectedCallback.Unregister();
#endif
}

USteamProRemotePlay* USteamProRemotePlay::GetSteamRemotePlay()
{
#if WITH_STEAMCORE
	return SteamRemotePlay() ? ThisClass::StaticClass()->GetDefaultObject<USteamProRemotePlay>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

int32 USteamProRemotePlay::GetSessionCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->GetSessionCount();
	}
#endif

	return Result;
}

int32 USteamProRemotePlay::GetSessionID(int32 SessionIndex)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->GetSessionID(SessionIndex);
	}
#endif

	return Result;
}

FSteamID USteamProRemotePlay::GetSessionSteamID(int32 SessionID)
{
	LogSteamCoreVeryVerbose("");

	FSteamID Result;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->GetSessionSteamID(SessionID);
	}
#endif

	return Result;
}

FString USteamProRemotePlay::GetSessionClientName(int32 SessionID)
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = TCHAR_TO_UTF8(SteamRemotePlay()->GetSessionClientName(SessionID));
	}
#endif

	return Result;
}

ESteamCoreProDeviceFormFactor USteamProRemotePlay::GetSessionClientFormFactor(int32 SessionID)
{
	LogSteamCoreVeryVerbose("");

	ESteamCoreProDeviceFormFactor Result = ESteamCoreProDeviceFormFactor::Unknown;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = static_cast<ESteamCoreProDeviceFormFactor>(SteamRemotePlay()->GetSessionClientFormFactor(SessionID));
	}
#endif

	return Result;
}

bool USteamProRemotePlay::BGetSessionClientResolution(int32 SessionID, int32& ResolutionX, int32& ResolutionY)
{
	LogSteamCoreVeryVerbose("");

	bool Result = false;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->BGetSessionClientResolution(SessionID, &ResolutionX, &ResolutionY);
	}
#endif

	return Result;
}

bool USteamProRemotePlay::BStartRemotePlayTogether(bool bShowOverlay)
{
	LogSteamCoreVerbose("");

	bool Result = false;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->BStartRemotePlayTogether(bShowOverlay);
	}
#endif

	return Result;
}

bool USteamProRemotePlay::BSendRemotePlayTogetherInvite(FSteamID SteamIDFriend)
{
	LogSteamCoreVeryVerbose("");

	bool Result = false;

#if WITH_STEAMCORE
	if (SteamRemotePlay())
	{
		Result = SteamRemotePlay()->BSendRemotePlayTogetherInvite(SteamIDFriend);
	}
#endif

	return Result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProRemotePlay::OnSteamRemotePlaySessionConnected(SteamRemotePlaySessionConnected_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamRemotePlaySessionConnected.Broadcast(Data);
	});
}

void USteamProRemotePlay::OnSteamRemotePlaySessionDisconnected(SteamRemotePlaySessionDisconnected_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamRemotePlaySessionDisconnected.Broadcast(Data);
	});
}
#endif
