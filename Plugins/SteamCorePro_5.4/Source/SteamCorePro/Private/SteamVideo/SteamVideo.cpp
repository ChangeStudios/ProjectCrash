/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamVideo
*/

#include "SteamVideo/SteamVideo.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProVideo::USteamProVideo()
{
#if WITH_STEAMCORE
	OnGetOPFSettingsResultCallback.Register(this, &USteamProVideo::OnGetOPFSettingsResult);
	OnGetVideoURLResultCallback.Register(this, &USteamProVideo::OnGetVideoURLResult);

	if (IsRunningDedicatedServer())
	{
		OnGetOPFSettingsResultCallback.SetGameserverFlag();
		OnGetVideoURLResultCallback.SetGameserverFlag();
	}
#endif
}

USteamProVideo::~USteamProVideo()
{
#if WITH_STEAMCORE
	OnGetOPFSettingsResultCallback.Unregister();
	OnGetVideoURLResultCallback.Unregister();
#endif
}

USteamProVideo* USteamProVideo::GetSteamVideo()
{
#if WITH_STEAMCORE
	return SteamVideo() ? ThisClass::StaticClass()->GetDefaultObject<USteamProVideo>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
void USteamProVideo::GetOPFSettings(int32 VideoAppID)
{
	LogSteamCoreVeryVerbose("");

#if WITH_STEAMCORE
	if (SteamVideo())
	{
		SteamVideo()->GetOPFSettings(VideoAppID);
	}
#endif
}

bool USteamProVideo::GetOPFStringForApp(int32 VideoAppID, FString& OutBuffer)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutBuffer.Empty();

#if WITH_STEAMCORE
	if (SteamVideo())
	{
		int32 BufferSize = 0;
		TArray<char> Buffer;
		Buffer.SetNum(512);

		bResult = SteamVideo()->GetOPFStringForApp(VideoAppID, Buffer.GetData(), &BufferSize);

		if (bResult)
		{
			Buffer.SetNum(BufferSize);
			OutBuffer = TCHAR_TO_UTF8(Buffer.GetData());
		}
	}
#endif

	return bResult;
}

void USteamProVideo::GetVideoURL(int32 VideoAppID)
{
	LogSteamCoreVeryVerbose("");

#if WITH_STEAMCORE
	if (SteamVideo())
	{
		SteamVideo()->GetVideoURL(VideoAppID);
	}
#endif
}

bool USteamProVideo::IsBroadcasting(int32& NumViewers)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamVideo())
	{
		bResult = SteamVideo()->IsBroadcasting(&NumViewers);
	}
#endif

	return bResult;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProVideo::OnGetOPFSettingsResult(GetOPFSettingsResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GetOPFSettingsResult.Broadcast(Data);
	});
}

void USteamProVideo::OnGetVideoURLResult(GetVideoURLResult_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GetVideoURLResult.Broadcast(Data);
	});
}
#endif
