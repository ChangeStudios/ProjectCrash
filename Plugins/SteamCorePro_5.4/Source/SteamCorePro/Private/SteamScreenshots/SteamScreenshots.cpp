/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamScreenshots
*/

#include "SteamScreenshots/SteamScreenshots.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProScreenshots::USteamProScreenshots()
{
#if WITH_STEAMCORE
	OnScreenshotReadyCallback.Register(this, &USteamProScreenshots::OnScreenshotReady);
	OnScreenshotRequestedCallback.Register(this, &USteamProScreenshots::OnScreenshotRequested);

	if (IsRunningDedicatedServer())
	{
		OnScreenshotReadyCallback.SetGameserverFlag();
		OnScreenshotRequestedCallback.SetGameserverFlag();
	}
#endif
}

USteamProScreenshots::~USteamProScreenshots()
{
#if WITH_STEAMCORE
	OnScreenshotReadyCallback.Unregister();
	OnScreenshotRequestedCallback.Unregister();
#endif
}

USteamProScreenshots* USteamProScreenshots::GetSteamScreenshots()
{
#if WITH_STEAMCORE
	return SteamScreenshots() ? ThisClass::StaticClass()->GetDefaultObject<USteamProScreenshots>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

FScreenshotHandle USteamProScreenshots::AddScreenshotToLibrary(FString FileName, FString ThumbnailFilename, int32 Width, int32 Height)
{
	LogSteamCoreVerbose("");

	FScreenshotHandle Handle;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		const FTCHARToUTF8 ConvertedFileName(*FileName);
		const FTCHARToUTF8 ConvertedThumbnailFileName(*ThumbnailFilename);
		
		Handle = SteamScreenshots()->AddScreenshotToLibrary(ConvertedFileName.Get(), ConvertedThumbnailFileName.Get(), Width, Height);
	}
#endif

	return Handle;
}

FScreenshotHandle USteamProScreenshots::AddVRScreenshotToLibrary(ESteamVRScreenshotType EType, FString FileName, FString VRFileName)
{
	LogSteamCoreVerbose("");

	FScreenshotHandle Handle;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		const FTCHARToUTF8 FileNameAnsi(*FileName);
		const FTCHARToUTF8 VrFileNameAnsi(*VRFileName);

		Handle = SteamScreenshots()->AddVRScreenshotToLibrary(static_cast<EVRScreenshotType>(EType), FileNameAnsi.Get(), VrFileNameAnsi.Get());
	}
#endif

	return Handle;
}

void USteamProScreenshots::HookScreenshots(bool bHook)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		SteamScreenshots()->HookScreenshots(bHook);
	}
#endif
}

bool USteamProScreenshots::IsScreenshotsHooked()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		bResult = SteamScreenshots()->IsScreenshotsHooked();
	}
#endif

	return bResult;
}

bool USteamProScreenshots::SetLocation(FScreenshotHandle Handle, FString Location)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		bResult = SteamScreenshots()->SetLocation(Handle, TCHAR_TO_UTF8(*Location));
	}
#endif

	return bResult;
}

bool USteamProScreenshots::TagPublishedFile(FScreenshotHandle Handle, FPublishedFileID PublishedFileID)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		bResult = SteamScreenshots()->TagPublishedFile(Handle, PublishedFileID);
	}
#endif

	return bResult;
}

bool USteamProScreenshots::TagUser(FScreenshotHandle Handle, FSteamID SteamID)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		bResult = SteamScreenshots()->TagUser(Handle, SteamID);
	}
#endif

	return bResult;
}

void USteamProScreenshots::TriggerScreenshot()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		SteamScreenshots()->TriggerScreenshot();
	}
#endif
}

FScreenshotHandle USteamProScreenshots::WriteScreenshot(TArray<uint8> PubRGB, int32 Width, int32 Height)
{
	LogSteamCoreVerbose("");

	FScreenshotHandle Handle;

#if WITH_STEAMCORE
	if (SteamScreenshots())
	{
		Handle = SteamScreenshots()->WriteScreenshot(PubRGB.GetData(), PubRGB.Num(), Width, Height);
	}
#endif

	return Handle;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProScreenshots::OnScreenshotReady(ScreenshotReady_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ScreenshotReady.Broadcast(Data);
	});
}

void USteamProScreenshots::OnScreenshotRequested(ScreenshotRequested_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		ScreenshotRequested.Broadcast(Data);
	});
}
#endif
