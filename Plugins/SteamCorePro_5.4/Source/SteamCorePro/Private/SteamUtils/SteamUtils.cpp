/**
* Copyright (C) 2017-2024 eelDev AB
*
* Official Steamworks Documentation: https://partner.steamgames.com/doc/api/ISteamUtils
*/

#include "SteamUtils/SteamUtils.h"
#include "SteamCoreProPluginPrivatePCH.h"

USteamProUtils::USteamProUtils()
{
#if WITH_STEAMCORE
	OnCheckFileSignatureCallback.Register(this, &USteamProUtils::OnCheckFileSignature);
	OnGamepadTextInputDismissedCallback.Register(this, &USteamProUtils::OnGamepadTextInputDismissed);
	OnLowBatteryPowerCallback.Register(this, &USteamProUtils::OnLowBatteryPower);
	OnIPCountryCallback.Register(this, &USteamProUtils::OnIPCountry);
	OnSteamShutdownCallback.Register(this, &USteamProUtils::OnSteamShutdown);
	OnAppResumingFromSuspendCallback.Register(this, &USteamProUtils::OnAppResumingFromSuspend);
	OnFloatingGamepadTextInputDismissedCallback.Register(this, &USteamProUtils::OnFloatingGamepadTextInputDismissed);

	if (IsRunningDedicatedServer())
	{
		OnCheckFileSignatureCallback.SetGameserverFlag();
		OnGamepadTextInputDismissedCallback.SetGameserverFlag();
		OnLowBatteryPowerCallback.SetGameserverFlag();
		OnIPCountryCallback.SetGameserverFlag();
		OnSteamShutdownCallback.SetGameserverFlag();
		OnAppResumingFromSuspendCallback.SetGameserverFlag();
		OnFloatingGamepadTextInputDismissedCallback.SetGameserverFlag();
	}
#endif
}

USteamProUtils::~USteamProUtils()
{
#if WITH_STEAMCORE
	OnCheckFileSignatureCallback.Unregister();
	OnGamepadTextInputDismissedCallback.Unregister();
	OnLowBatteryPowerCallback.Unregister();
	OnIPCountryCallback.Unregister();
	OnSteamShutdownCallback.Unregister();
	OnAppResumingFromSuspendCallback.Unregister();
	OnFloatingGamepadTextInputDismissedCallback.Unregister();
#endif
}

USteamProUtils* USteamProUtils::GetSteamUtils()
{
#if WITH_STEAMCORE
	return SteamUtils() ? ThisClass::StaticClass()->GetDefaultObject<USteamProUtils>() : nullptr;
#endif
	return nullptr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
int32 USteamProUtils::GetSecondsSinceAppActive()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetSecondsSinceAppActive();	
	}
#endif

	return Result;
}

int32 USteamProUtils::GetSecondsSinceComputerActive()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetSecondsSinceComputerActive();	
	}
#endif
	
	return Result;
}

ESteamUniverse USteamProUtils::GetConnectedUniverse()
{
	LogSteamCoreVeryVerbose("");

	ESteamUniverse Result = ESteamUniverse::Invalid;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = static_cast<ESteamUniverse>(GetUtils()->GetConnectedUniverse());
	}
#endif
	
	return Result;
}

int32 USteamProUtils::GetServerRealTime()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetServerRealTime();
	}
#endif

	return Result;
}

FString USteamProUtils::GetIPCountry()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = UTF8_TO_TCHAR(GetUtils()->GetIPCountry());
	}
#endif

	return Result;
}

int32 USteamProUtils::GetCurrentBatteryPower()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetCurrentBatteryPower();
	}
#endif

	return Result;
}

int32 USteamProUtils::GetAppID()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetAppID();
	}
#endif

	return Result;
}

void USteamProUtils::SetOverlayNotificationPosition(ESteamNotificationPosition NotificationPosition)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetUtils())
	{
		GetUtils()->SetOverlayNotificationPosition(static_cast<ENotificationPosition>(NotificationPosition));
	}
#endif
}

void USteamProUtils::SetVRHeadsetStreamingEnabled(bool bEnabled)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetUtils())
	{
		GetUtils()->SetVRHeadsetStreamingEnabled(bEnabled);
	}
#endif
}

int32 USteamProUtils::GetIPCCallCount()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetIPCCallCount();
	}
#endif

	return Result;
}

bool USteamProUtils::IsOverlayEnabled()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsOverlayEnabled();
	}
#endif

	return bResult;
}

bool USteamProUtils::IsSteamChinaLauncher()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsSteamChinaLauncher();
	}
#endif

	return bResult;
}

bool USteamProUtils::InitFilterText()
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->InitFilterText();
	}
#endif

	return bResult;
}

int32 USteamProUtils::FilterText(ESteamTextFilteringContext Context, FSteamID SourceSteamID, FString InputMessage, FString& OutFilteredText)
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;
	OutFilteredText.Reset(InputMessage.Len());

#if WITH_STEAMCORE
	if (GetUtils())
	{
		uint32 nByteSizeOutFilteredText = InputMessage.Len() * sizeof(InputMessage);
		char* pchOutFilteredText = new char[nByteSizeOutFilteredText];
		const FTCHARToUTF8 ConvertedInputMessage(*InputMessage);
		Result = GetUtils()->FilterText(static_cast<ETextFilteringContext>(Context), SourceSteamID, ConvertedInputMessage.Get(), pchOutFilteredText, nByteSizeOutFilteredText);
		const FUTF8ToTCHAR ConvertedOutFilteredText(pchOutFilteredText);
		OutFilteredText = ConvertedOutFilteredText.Get();
		delete[] pchOutFilteredText;
	}
#endif

	return Result;
}

bool USteamProUtils::IsSteamRunningOnSteamDeck()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsSteamRunningOnSteamDeck();
	}
#endif

	return bResult;
}

bool USteamProUtils::BOverlayNeedsPresent()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->BOverlayNeedsPresent();
	}
#endif

	return bResult;
}

bool USteamProUtils::ShowGamepadTextInput(ESteamGamepadTextInputMode InputMode, ESteamGamepadTextInputLineMode LineInputMode, FString Description, int32 CharMax, FString ExistingText)
{
	LogSteamCoreVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->ShowGamepadTextInput(static_cast<EGamepadTextInputMode>(InputMode), static_cast<EGamepadTextInputLineMode>(LineInputMode), TCHAR_TO_UTF8(*Description), CharMax, TCHAR_TO_UTF8(*ExistingText));
	}
#endif

	return bResult;
}

int32 USteamProUtils::GetEnteredGamepadTextLength()
{
	LogSteamCoreVeryVerbose("");

	int32 Result = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->GetEnteredGamepadTextLength();
	}
#endif

	return Result;
}

bool USteamProUtils::GetImageRGBA(int iImage, TArray<uint8>& OutBuffer)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutBuffer.Empty();

#if WITH_STEAMCORE
	if (GetUtils())
	{
		uint32 AvatarWidth = 0;
		uint32 AvatarHeight = 0;

		if (GetUtils()->GetImageSize(iImage, &AvatarWidth, &AvatarHeight))
		{
			const int ImageSizeInBytes = AvatarWidth * AvatarHeight * 4;
			OutBuffer.SetNum(ImageSizeInBytes);

			bResult = GetUtils()->GetImageRGBA(iImage, OutBuffer.GetData(), ImageSizeInBytes);
		}
	}
#endif

	return bResult;
}

bool USteamProUtils::GetImageSize(int iImage, int32& OutWidth, int32& OutHeight)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	uint32 Width = 0;
	uint32 Height = 0;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->GetImageSize(iImage, &Width, &Height);
	}
#endif

	OutWidth = Width;
	OutHeight = Height;

	return bResult;
}

bool USteamProUtils::GetEnteredGamepadTextInput(FString& OutText)
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;
	OutText.Empty();

#if WITH_STEAMCORE
	if (GetUtils())
	{
		TArray<char> DataArray;
		uint32 Length = GetUtils()->GetEnteredGamepadTextLength();
		LogSteamCoreVerbose("GetEnteredGamepadTextLength (%i)", Length);

		DataArray.SetNum(Length);
		bResult = GetUtils()->GetEnteredGamepadTextInput(DataArray.GetData(), Length);
		LogSteamCoreVerbose("GetEnteredGamepadTextInput result: (%d), DataArray length: (%i)", bResult, DataArray.Num());

		if (bResult)
		{
			OutText = UTF8_TO_TCHAR(DataArray.GetData());
		}
	}
#endif

	return bResult;
}

FString USteamProUtils::GetSteamUILanguage()
{
	LogSteamCoreVeryVerbose("");

	FString Result;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = UTF8_TO_TCHAR(GetUtils()->GetSteamUILanguage());
	}
#endif

	return Result;
}

bool USteamProUtils::IsSteamRunningInVR()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsSteamRunningInVR();
	}
#endif

	return bResult;
}

bool USteamProUtils::IsVRHeadsetStreamingEnabled()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsVRHeadsetStreamingEnabled();
	}
#endif

	return bResult;
}

void USteamProUtils::SetOverlayNotificationInset(int32 HorizontalInset, int32 VerticalInset)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetUtils())
	{
		GetUtils()->SetOverlayNotificationInset(HorizontalInset, VerticalInset);
	}
#endif
}

bool USteamProUtils::IsSteamInBigPictureMode()
{
	LogSteamCoreVeryVerbose("");

	bool bResult = false;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		bResult = GetUtils()->IsSteamInBigPictureMode();
	}
#endif

	return bResult;
}

void USteamProUtils::StartVRDashboard()
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetUtils())
	{
		GetUtils()->StartVRDashboard();
	}
#endif
}

ESteamCoreIPv6ConnectivityState USteamProUtils::GetIPv6ConnectivityState(ESteamCoreIPv6ConnectivityProtocol Protocol)
{
	LogSteamCoreVerbose("");
	
	ESteamCoreIPv6ConnectivityState Result = ESteamCoreIPv6ConnectivityState::k_ESteamIPv6ConnectivityState_Unknown;

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = static_cast<ESteamCoreIPv6ConnectivityState>(GetUtils()->GetIPv6ConnectivityState(static_cast<ESteamIPv6ConnectivityProtocol>(Protocol)));
	}
#endif
		
	return Result;
}

bool USteamProUtils::ShowFloatingGamepadTextInput(ESteamFloatingGamepadTextInputMode KeyboardMode, int32 TextFieldXPosition, int32 TextFieldYPosition, int32 TextFieldWidth, int32 TextFieldHeight)
{
	LogSteamCoreVerbose("");
	
	bool Result = false; 

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->ShowFloatingGamepadTextInput(static_cast<EFloatingGamepadTextInputMode>(KeyboardMode), TextFieldXPosition, TextFieldYPosition, TextFieldWidth, TextFieldHeight);
	}
#endif
		
	return Result; 
}

void USteamProUtils::SetGameLauncherMode(bool bLauncherMode)
{
	LogSteamCoreVerbose("");

#if WITH_STEAMCORE
	if (GetUtils())
	{
		GetUtils()->SetGameLauncherMode(bLauncherMode);
	}
#endif
}

bool USteamProUtils::DismissFloatingGamepadTextInput()
{
	LogSteamCoreVerbose("");
	
	bool Result = false; 

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->DismissFloatingGamepadTextInput();
	}
#endif
		
	return Result; 
}

bool USteamProUtils::DismissGamepadTextInput()
{
	LogSteamCoreVerbose("");
	
	bool Result = false; 

#if WITH_STEAMCORE
	if (GetUtils())
	{
		Result = GetUtils()->DismissGamepadTextInput();
	} 
#endif
		
	return Result; 
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//		Steam API Callbacks
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#if WITH_STEAMCORE
void USteamProUtils::OnCheckFileSignature(CheckFileSignature_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		CheckFileSignature.Broadcast(Data);
	});
}

void USteamProUtils::OnGamepadTextInputDismissed(GamepadTextInputDismissed_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		GamepadTextInputDismissed.Broadcast(Data);
	});
}

void USteamProUtils::OnLowBatteryPower(LowBatteryPower_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		LowBatteryPower.Broadcast(Data);
	});
}

void USteamProUtils::OnIPCountry(IPCountry_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		IPCountry.Broadcast(Data);
	});
}

void USteamProUtils::OnSteamShutdown(SteamShutdown_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		SteamShutdown.Broadcast(Data);
	});
}


void USteamProUtils::OnAppResumingFromSuspend(AppResumingFromSuspend_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		AppResumingFromSuspend.Broadcast(Data);
	});
}

void USteamProUtils::OnFloatingGamepadTextInputDismissed(FloatingGamepadTextInputDismissed_t* pParam)
{
	LogSteamCoreVerbose("");

	auto Data = *pParam;
	AsyncTask(ENamedThreads::GameThread, [this, Data]()
	{
		FloatingGamepadTextInputDismissed.Broadcast(Data);
	});
}
#endif
